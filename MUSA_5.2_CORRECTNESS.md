# MUSA 5.2 Goldilocks NTT correctness receipt

Date: 2026-08-04 (Asia/Shanghai)

Task: `5a228815-fccf-4dd2-83ec-57e1187a4897`

Base: exact reviewed S2 result
`c0bf2d8490d77ef89a0d4d1de3926c75529f3d00`

## Decision

**GO for the bounded native-MUSA Goldilocks NTT denominator.** The authentic
`poc/ntt-cuda/cuda/ntt_api.cu` production path executed `compute_ntt` on the
assigned MTT S5000. All 1,048,576 mapped outputs exactly matched a frozen,
independent CPU mathematical oracle modulo `0xffffffff00000001`; every output
was canonical, the call and process exited successfully, and concurrent device
sampling observed GPU work.

This establishes MUSA 5.2 correctness only for the receipt's Goldilocks,
forward, standard, NN-order, `2^20` transform on S5000/mp_31. It does not claim
inverse/coset/other-order correctness, larger fields, MSM, performance, or CUDA
and ROCm regression coverage.

## Reviewed start and resource mapping

The S0-S2 receipts (`MUSA_5.2_SCOPE.md`, `MUSA_5.2_IMPLEMENTATION.md`, and
`MUSA_5.2_GOLDILOCKS.md`) were read completely before work. No separate review
disposition existed in the worktree, Git refs/notes, or nearby files, matching
the review-payload absence already recorded by S1. Initial state was detached,
clean `HEAD` at the exact base above.

The resource contract assigned physical GPU index 1. The container exposed:

```text
MTHREADS_VISIBLE_DEVICES=1
/dev/mtgpu.1
```

MUSA therefore saw one device at its sole container-local ordinal 0. Only the
capabilities required by this path were checked. `/usr/local/musa/bin/musaInfo`
reported:

```text
device# 0
Name: MTT S5000
Driver Version: 5.2
Runtime Version: 5.2
compute capability: 3.1
multiProcessorCount: 56
sharedMemPerBlock: 192.00 KB
warpSize: 32
maxThreadsPerBlock: 1024
cooperativeLaunch: 1
memInfo.total: 79.91 GB
```

These establish the fixed 32-lane shuffle assumption, `mp_31` target, launch
limits, and cooperative-launch gate reached by SPPARK. Async allocation,
streams, copies, twiddle kernels, and NTT launches were then exercised by the
production call itself rather than by adding synthetic gates.

Two harmless operator probes failed and were not treated as validation runs:
an unsupported selective-query field (`pci.bus_id`) was rejected, and
`mthreads-gmi -i 1 -cf` could not find local index 1. The latter is consistent
with physical index 1 being isolated as local ordinal 0. No kernel was launched
by either probe.

## Authentic build and backend identity

The existing S2 production translation unit was compiled directly; no
runtime-only kernel was substituted:

```sh
OMP_NUM_THREADS=2 timeout 600s /usr/local/musa/bin/mcc \
  -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -DFEATURE_GOLDILOCKS \
  -DGL64_NO_REDUCTION_KLUDGE -fPIC \
  -c poc/ntt-cuda/cuda/ntt_api.cu \
  -o target/s3-build/ntt_api.o

OMP_NUM_THREADS=2 timeout 300s /usr/local/musa/bin/mcc \
  -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -fPIC \
  -c util/all_gpus.cpp -o target/s3-build/all_gpus.o

/usr/local/musa/bin/mcc -shared target/s3-build/ntt_api.o \
  target/s3-build/all_gpus.o -L/usr/local/musa/lib \
  -Wl,-rpath,/usr/local/musa/lib -lmusart \
  -o target/s3-build/libntt_gl64_musa.so
```

`nm` showed exported `compute_ntt` and `cuda_available`. `readelf`/`ldd` showed
`libmusart.so.5` resolved from `/usr/local/musa/lib`, with no CUDA or ROCm GPU
runtime. The MUSA preprocessing pass defined `__MUSACC__` and
`__MUSA_ARCH__=310`; it defined none of `__CUDACC__`, `__NVCC__`,
`__CUDA_ARCH__`, or `__HIPCC__`. Thus mcc received neither PTX/GCN arithmetic
nor fake NVIDIA identity.

Tool identity was mcc 5.2.0 (Clang 14), MUSA toolkit/runtime 5.2.0, toolkit tag
`20260602_master`, commit `8c44cbed02a79be2066f1f62bdca039177f71d51`.

## Frozen workload and independent oracle

Before the retained run, `tests/musa_gl64_ntt_correctness.cpp` and its compiled
executable were frozen and hashed. The test uses only the `compute_ntt` ABI; it
does not include SPPARK field, root, parameter, or NTT headers.

Frozen transform:

| Property | Exact value |
|---|---|
| Field modulus | `p = 0xffffffff00000001` |
| Domain | `lg_n = 20`, `n = 1,048,576` |
| Device mapping | physical 1, sole local ordinal `device_id = 0` |
| API semantics | `order=NN (0)`, `direction=forward (0)`, `type=standard (0)` |
| Primitive generator | canonical Goldilocks generator `7` |
| Input seed | `0x5a228815fccf4dd2` |
| Input definition | `splitmix64(seed+i) mod p`; indices 0..3 replaced by `0, 1, p-1, p-2` |
| Output mapping | natural-order `actual[k]` compared exactly to natural-order CPU `expected[k]` for every `k` |
| Success | `compute_ntt.code == 0`, zero noncanonical values, zero exact mismatches, process exit 0 |

The CPU oracle independently computes
`root = 7^((p-1)/n) mod p`, bit-reverses the input, and performs an iterative
radix-2 Cooley-Tukey transform. Its addition, subtraction, multiplication, and
exponentiation use separate host `unsigned __int128 % p` arithmetic, not the
MUSA Goldilocks reduction or upstream root table. Comparison is exact on the
canonical residues, which is stricter than residue equivalence alone.

Freeze hashes:

```text
e33cfca52e5d77c3eb24f37375919d22fd8da97fbefe2917502bb4bc8c8a5f94  tests/musa_gl64_ntt_correctness.cpp
101c88b2e67de67a90b07193c42f49ea8c139568ceea2f49857eec44aef1d588  target/s3-build/musa_gl64_ntt_correctness
cf0c6434462ed472e1d2d94fddc467efa4ac75242f2776bacb22e67b6bf33fbb  target/s3-build/libntt_gl64_musa.so
```

No `compute_ntt` debugging or rehearsal call occurred before this freeze.

## Single retained validation run

Concurrent activity capture began 200 ms before the one retained executable
invocation and sampled local S5000 ordinal 0 every 100 ms:

```sh
timeout 20s stdbuf -oL mthreads-gmi \
  --query-gpu=timestamp,index,name,utilization.gpu,utilization.memory,memory.used \
  --format=csv,noheader -i 0 -lms 100 \
  > target/s3-evidence/utilization.csv 2>&1 &

target/s3-build/musa_gl64_ntt_correctness \
  target/s3-build/libntt_gl64_musa.so \
  2>&1 | tee target/s3-evidence/retained-run.txt
```

Exact retained output:

```text
backend=MUSA device_local=0 lg_n=20 n=1048576 order=NN direction=forward type=standard
modulus=ffffffff00000001 generator=7 seed=5a228815fccf4dd2 input_fn=splitmix64(seed+i)_mod_p_with_boundary_prefix
input_fnv1a64=18fcc3fe018b8068 oracle_fnv1a64=2174821b208fa824 gpu_fnv1a64=2174821b208fa824
exit_status=0 exact_mismatches=0 noncanonical=0
retained_run_exit=0
monitor_timeout_exit=124
```

The monitor produced 194 samples. Peak observed GPU utilization was 1%, peak
allocated device memory was 35 MiB, and this nonzero sample overlapped the run:

```text
2026/08/04 00:32:10.662, 0, MTT S5000, 1 [%], 0 [%], 0 [MiB]
```

The monitor's exit 124 is the planned 20-second capture timeout after the NTT
had completed; it is not a validation failure. The retained NTT process exited
0. No CPU, CUDA, or ROCm fallback exists in the loaded production library.

## Remaining support boundary

- This is real correctness evidence for the frozen Goldilocks denominator,
  not broad SPPARK or MUSA performance evidence.
- Inverse, coset, NR/RN/RR order combinations and other sizes still need
  independent-oracle coverage before their correctness can be claimed.
- Multi-limb fields and MSM still require MUSA arithmetic work and independent
  validation; the NVIDIA PTX and AMD GCN implementations remain backend-only.
- CUDA and ROCm source paths were preserved unchanged. Their compilers were
  absent, so compile/runtime regressions remain inconclusive.
- Cargo was not required for this ABI-level denominator: the test loaded and
  called the exact production `compute_ntt` symbol compiled from the existing
  PoC translation unit.
