# SPPARK MUSA 5.2 support and maintenance handoff

Date: 2026-08-04 (Asia/Shanghai)

This document closes the accepted S0-S3 MUSA work and is the support boundary
for this source line. It does not replace the stage receipts; those remain the
auditable evidence record.

## Frozen identities and lineage

- Canonical source: `git@github.com:supranational/sppark.git`.
- Upstream source: `17278d74295392f9813f009300b257a688422b7a`
  (`v0.1.15`, `rust/Cargo.toml: bump the version number.`).
- Accepted S0 result: `763ede6abb3f83cb8d20f2ff95e1fd25c251aae7`.
  Its initial receipt commit is
  `07b29c42d289b360096d2493565a9f5c8031ba9e`; `763ede6` is its formatting-only
  child.
- Accepted S1 result: `95e27ad36588e67986a8e9b51d180a6452ab719a`.
- Accepted S2 result: `c0bf2d8490d77ef89a0d4d1de3926c75529f3d00`.
- Accepted S3 correctness result:
  `5f70f1724d612a5b47fb4d07798de7086af5500e`.
- Lineage is linear:
  `17278d7 -> 07b29c4 -> 763ede6 -> 95e27ad -> c0bf2d8 -> 5f70f17`.

The accepted compiler/toolkit identity is Linux x86-64, mcc 5.2.0 based on
Clang 14, MUSA toolkit/runtime 5.2.0, toolkit tag `20260602_master`, toolkit
commit `8c44cbed02a79be2066f1f62bdca039177f71d51`, and explicit architecture
`mp_31`. The validated device was an MTT S5000 with driver/runtime 5.2, compute
capability 3.1, warp size 32, and cooperative launch support.

## Accepted receipts and dispositions

| Stage | Durable receipt | SHA-256 | Accepted disposition |
|---|---|---|---|
| S0 | `MUSA_5.2_SCOPE.md` | `ddfd5f89358f2f6430b4367314d9657c532e1704753da31e9623fff911d79e3a` | Audit/plan was conclusive; unmodified source was NO_GO for MUSA compilation and GPU support was INCONCLUSIVE. |
| S1 | `MUSA_5.2_IMPLEMENTATION.md` | `5f3b4351d634c1275c25cb03b91110a3d19cba30dc6d6b5f66a309172cbae812` | PASS for bounded plumbing compile/link; runtime, arithmetic, correctness, and performance remained INCONCLUSIVE. |
| S2 | `MUSA_5.2_GOLDILOCKS.md` | `2749b4c9abac468c821976502208fac674d36282004aa59c686cf25991de192f` | PASS for portable Goldilocks host tests and authentic NTT compile/link; correctness and GPU support remained INCONCLUSIVE. |
| S3 | `MUSA_5.2_CORRECTNESS.md` | `0068fcc432d1521a8cd8a771f550a6cdcf774b7152ec22966cb608568de57bac` | GO only for the frozen native-MUSA Goldilocks denominator below. |

S1 records review identifier `b235b48a-63f7-4b75-b7a9-97e86f46079f` but no
payload was present in the worktree, Git refs/notes, environment, or configured
resources. S3 repeated that search and found no separate review disposition.
Consequently, the accepted commits and the four receipts above are the complete
node-local review record; absence of another payload must not be interpreted as
an additional approval.

## Supported denominator

The support claim is exactly the authentic production `compute_ntt` path built
from `poc/ntt-cuda/cuda/ntt_api.cu` with:

- backend MUSA 5.2 and architecture `mp_31` on MTT S5000;
- Goldilocks field, modulus `p = 0xffffffff00000001`;
- `lg_n = 20`, hence exactly `n = 1,048,576` (`2^20`) field elements;
- `order=NN (0)`, `direction=forward (0)`, `type=standard (0)`;
- natural-order output and canonical residues.

S3's retained call returned code 0 and process exit 0. All 1,048,576 outputs
matched exactly, with zero noncanonical values and zero mismatches. Its output
digests were:

```text
input_fnv1a64=18fcc3fe018b8068
oracle_fnv1a64=2174821b208fa824
gpu_fnv1a64=2174821b208fa824
```

The frozen input is `splitmix64(0x5a228815fccf4dd2 + i) mod p`, with indices
0..3 replaced by `0, 1, p-1, p-2`. The independent CPU oracle uses generator
7, computes `root = 7^((p-1)/n) mod p`, bit-reverses the input, then runs an
iterative radix-2 Cooley-Tukey forward transform. Its arithmetic is host
`unsigned __int128 % p`; it includes no SPPARK field, root, parameter, or NTT
header. The frozen oracle source hash is
`e33cfca52e5d77c3eb24f37375919d22fd8da97fbefe2917502bb4bc8c8a5f94`.

The retained binary hashes were:

```text
101c88b2e67de67a90b07193c42f49ea8c139568ceea2f49857eec44aef1d588  musa_gl64_ntt_correctness
cf0c6434462ed472e1d2d94fddc467efa4ac75242f2776bacb22e67b6bf33fbb  libntt_gl64_musa.so
```

## Reproducible build shape

Run from the repository root with at most the assigned resources. The accepted
production build was:

```sh
export OMP_NUM_THREADS=2
mkdir -p target/s3-build
timeout 600s /usr/local/musa/bin/mcc \
  -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -DFEATURE_GOLDILOCKS \
  -DGL64_NO_REDUCTION_KLUDGE -fPIC \
  -c poc/ntt-cuda/cuda/ntt_api.cu -o target/s3-build/ntt_api.o
timeout 300s /usr/local/musa/bin/mcc \
  -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -fPIC \
  -c util/all_gpus.cpp -o target/s3-build/all_gpus.o
/usr/local/musa/bin/mcc -shared \
  target/s3-build/ntt_api.o target/s3-build/all_gpus.o \
  -L/usr/local/musa/lib -Wl,-rpath,/usr/local/musa/lib -lmusart \
  -o target/s3-build/libntt_gl64_musa.so
```

The receipt verified exported `compute_ntt` and `cuda_available`, and resolved
`libmusart.so.5` from `/usr/local/musa/lib` with no CUDA or ROCm runtime. The
MUSA pass defined `__MUSACC__` and `__MUSA_ARCH__=310`, but none of
`__CUDACC__`, `__NVCC__`, `__CUDA_ARCH__`, or `__HIPCC__`.

The retained GPU command is evidence, not a routine smoke test:

```sh
target/s3-build/musa_gl64_ntt_correctness \
  target/s3-build/libntt_gl64_musa.so
```

Do not replay it during ordinary maintenance. It requires an explicitly
assigned compatible GPU and a validation plan that preserves the executable,
library, output, device activity, and hashes. Compile the independent driver,
when an authorized revalidation needs a new one, with:

```sh
g++ -std=c++17 -O2 tests/musa_gl64_ntt_correctness.cpp \
  -ldl -o target/s3-build/musa_gl64_ntt_correctness
```

## Explicitly unvalidated modes and boundaries

No support claim is made for inverse transforms; cosets; NR, RN, or RR order;
other transform sizes; other MUSA architectures or devices; multi-limb or
other fields; MSM; performance; or CUDA/ROCm compilation, regression, runtime,
or correctness. Preservation of CUDA and ROCm branches is not validation.
Do not infer any of these claims from successful compilation or from a
forward/inverse round trip.

The MUSA implementation depends on the reviewed portable Goldilocks arithmetic
in `ff/gl64_t.musa`, backend identity in `util/gpu_backend.hpp`, runtime mapping
in `util/cuda2musa.hpp`, and the production NTT sources under `ntt/`. It requires
`mcc`, MUSA headers, and `libmusart.so.5`; `/usr/local/musa/bin` may not be on
`PATH`. Fixed 32-lane shuffle behavior, cooperative launch, stream/event and
async-allocation ordering, symbols, occupancy, and shared-memory limits are
device/runtime boundaries. S3 exercised those reached by this one production
call, not every API combination.

PTX in the CUDA field/MSM paths and AMD GCN code in the ROCm paths are not MUSA
implementations. BLST/Arkworks/semolina and Rust/Go toolchains are integration
or future-oracle dependencies, not part of the accepted ABI-level denominator.
The accepted Cargo MUSA plumbing was checked in S1, but Cargo was unavailable
for S2/S3 and was not needed for the retained correctness call.

## Periodic refresh procedure

Refresh quarterly and whenever the upstream commit, MUSA toolkit/runtime,
driver, target architecture/device, compiler flags, portable Goldilocks code,
runtime mapping, NTT kernels/parameters, or `compute_ntt` ABI changes.

1. Create a clean detached worktree at the proposed upstream commit. Record
   the remote, full commit, parent/merge-base lineage, submodules, and clean
   status. Diff every path named in the dependency paragraph against this
   baseline; do not silently carry the support claim across a change.
2. Record `mcc --version`, `musa_toolkits_version`, toolkit tag/commit,
   driver/runtime, device model, compute capability, warp size, cooperative
   launch capability, and the explicit `--offload-arch`. A changed identity is
   a new validation target, not a maintenance-equivalent build.
3. With two CPU jobs, run `tests/check_gl64_selection.py` and the host executable
   from `tests/gl64_portable_test.cpp`. Then run
   `tests/check_musa_gl64_stage3.sh` as compile/link-only evidence. Inspect
   symbols, runtime dependencies, runpath, and prohibited vendor macros. These
   checks do not establish GPU correctness.
4. If and only if GPU revalidation is authorized and the exact device is
   assigned, freeze and hash the independent driver and shared library before
   the first `compute_ntt` call. Confirm local/physical device mapping. Perform
   one retained `2^20` call while capturing device activity; retain stdout,
   exit codes, exact mismatch/noncanonical counts, digests, and artifact hashes.
5. Compare every output to the independent CPU oracle, not to a round trip or
   another SPPARK implementation. Any mismatch, noncanonical output, nonzero
   call/process status, wrong runtime, missing device activity, or ambiguous
   device mapping is NO_GO. Missing tools/device/evidence is INCONCLUSIVE.
6. For any proposed expansion, add an independent oracle and a separate
   denominator for each direction, coset/order, size, field, or algorithm.
   Update this document only after that evidence receives formal acceptance.

For each refresh, retain a concise evidence checklist: source and parent
commits; clean status; receipt/source/artifact SHA-256 hashes; exact commands
and exit codes; compiler/toolkit/driver/runtime/device identities; architecture
flag and device mapping; exported symbols and dynamic dependencies; macro
selection; host-test results; frozen transform parameters and seed; oracle
method; input/oracle/device digests; mismatch and canonicality counts; device
activity; skipped checks with reasons; and an explicit supported/unvalidated
boundary. Never report GO when any required item is skipped or inconclusive.

## Final source-only verification

On 2026-08-04, source-only checks confirmed that every receipt, source, and
configuration path named above exists, that the embedded S0-S3 receipt hashes
and commit-parent lineage match the accepted records, and that the supported
Goldilocks forward/NN/standard `2^20` denominator remains separate from every
explicitly unvalidated mode. No GPU workload, Python check, or artifact-
generating command was run; retained binary and runtime-evidence paths remain
references to the accepted S3 record only.
