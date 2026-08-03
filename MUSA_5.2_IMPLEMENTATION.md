# MUSA 5.2 plumbing frontier receipt

Date: 2026-08-03 (Asia/Shanghai)

Task: `37d02016-a42b-46fa-adeb-321998a47ac0`

Base: accepted S0 result `763ede6abb3f83cb8d20f2ff95e1fd25c251aae7`

## Result and boundary

**INCONCLUSIVE overall. PASS for the bounded stage 1-2 compile/link smoke;
GPU runtime, arithmetic, correctness, and performance remain unestablished.**
No GPU was assigned and no GPU binary, availability probe, enumeration API, or
kernel was executed.

The accepted scope `MUSA_5.2_SCOPE.md` was read in full. The supplied accepted
review identifier `b235b48a-63f7-4b75-b7a9-97e86f46079f` had no payload in the
worktree, Git refs, environment, or configured resources, so no additional
review text was locally available.

This descendant adds only:

- explicit, mutually exclusive CUDA/ROCm/MUSA compiler selection in Rust and
  Go, with `MCC`, `MUSA_ARCH`, and `SPPARK_BACKEND=musa` controls;
- centralized compiler/device-pass identity without defining `__CUDACC__`,
  `__NVCC__`, or `__CUDA_ARCH__` for MUSA;
- a MUSA-only runtime spelling boundary limited to the APIs reached by
  `poc/go/poc.cu` and `util/all_gpus.cpp` through `util/gpu_t.cuh`;
- an explicit MUSA 5.2 `mp_31` consumer compiler path and `musart` linkage;
- focused C/C++ and Go selection checks plus a non-executing smoke script.

The CUDA and ROCm build branches remain present. Multiple explicit Rust
features fail at compile/build time. Go auto-discovery rejects multiple
installed GPU compilers and asks for one explicit `SPPARK_BACKEND` instead of
probing a GPU to choose between them.

## Tooling and resource envelope

All build commands used `OMP_NUM_THREADS=2`; Go checks also used
`GOMAXPROCS=2`. No GPU indices were assigned or accessed.

MUSA identity:

```text
clang version 14.0.0
mcc version 5.2.0
Target: x86_64-unknown-linux-gnu
musa_toolkits version: 5.2.0
toolkit tag: 20260602_master
toolkit commit: 8c44cbed02a79be2066f1f62bdca039177f71d51
```

Go was absent from the initial `PATH`. The official Go 1.26.5 Linux amd64
archive was downloaded to the ignored `target/toolchain-cache` area, verified
against the go.dev SHA-256
`5c2c3b16caefa1d968a94c1daca04a7ca301a496d9b086e17ad77bb81393f053`,
and extracted under ignored `target/toolchains`. No host package manager or
privileged install was used.

Rust was likewise absent. Official rustup installed the 2026-07-16 stable
toolchain into the same ignored area: `cargo 1.97.1` and `rustc 1.97.1`. Cargo
resolved the existing manifest through crates.io; no manifest or lockfile was
changed or committed.

## Exact compile/link evidence

The committed bounded check was run from the repository root:

```sh
OUTPUT_DIR=/tmp/sppark-musa-frontier-script \
OMP_NUM_THREADS=2 \
timeout 180s tests/check_musa_frontier.sh

readelf -d /tmp/sppark-musa-frontier-script/poc_musa.so |
  rg 'NEEDED|RUNPATH'
```

Result:

```text
MUSA compile/link frontier passed: /tmp/sppark-musa-frontier-script/poc_musa.so
Shared library: [libmusart.so.5]
Shared library: [libstdc++.so.6]
Shared library: [libgcc_s.so.1]
Shared library: [libc.so.6]
Library runpath: [/usr/local/musa/lib]
```

The script performs these bounded operations, without loading or executing the
result:

```sh
g++ -I. -D__MUSACC__ -D__HIPCC__ \
  -include util/gpu_backend.hpp -x c++ -fsyntax-only -
g++ -I. -D__MUSACC__ -include util/gpu_backend.hpp -x c++ -dM -E -
/usr/local/musa/bin/mcc -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -fPIC -c poc/go/poc.cu -o "$OUTPUT_DIR/poc.o"
/usr/local/musa/bin/mcc -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -fPIC -c util/all_gpus.cpp \
  -o "$OUTPUT_DIR/all_gpus.o"
/usr/local/musa/bin/mcc -shared "$OUTPUT_DIR/poc.o" \
  "$OUTPUT_DIR/all_gpus.o" -L/usr/local/musa/lib \
  -Wl,-rpath,/usr/local/musa/lib -lmusart -o "$OUTPUT_DIR/poc_musa.so"
```

`nm -D --defined-only` also showed exported `cuda_func` and `cuda_available`.
The compiler macro check found none of `__CUDACC__`, `__NVCC__`, or
`__CUDA_ARCH__` in the MUSA pass.

Focused Go selector tests were run independently of the full BLST-backed Go
package (the first module-proxy download attempt timed out):

```sh
mkdir -p target/go-tmp target/go-cache
OMP_NUM_THREADS=2 GOMAXPROCS=2 \
GOTMPDIR=/workspace/target/go-tmp GOCACHE=/workspace/target/go-cache \
timeout 120s target/toolchains/go/bin/go test \
  go/backend.go go/backend_test.go
```

Result: `ok command-line-arguments 0.002s`. The tests prove explicit CUDA,
ROCm, and MUSA selection, explicit MUSA selection in the presence of all three
compiler paths, automatic multi-compiler failure, and unknown-backend failure.

The module proxy timed out twice while fetching BLST. The official v0.3.16 tag
archive was therefore acquired directly from the upstream GitHub repository and
used from the ignored tool area through an untracked local modfile. This allowed
a full, non-running Go package compile:

```sh
OMP_NUM_THREADS=2 GOMAXPROCS=2 \
GOTMPDIR=/workspace/target/go-tmp GOCACHE=/workspace/target/go-cache \
GOMODCACHE=/workspace/target/go-mod GOPROXY=off \
PATH=/workspace/target/toolchains/go/bin:$PATH \
timeout 180s target/toolchains/go/bin/go test \
  -modfile=/workspace/target/go-local.mod -run '^$' ./go
```

Result: exit 0, `ok github.com/sppark/sppark/go 0.003s [no tests to run]`.

The real Rust MUSA build-script path was checked with the downloaded crates and
the assigned two-job limit:

```sh
OMP_NUM_THREADS=2 CARGO_BUILD_JOBS=2 \
CARGO_HOME=/workspace/target/toolchains/cargo \
RUSTUP_HOME=/workspace/target/toolchains/rustup \
CARGO_TARGET_DIR=/workspace/target/rust-check \
MCC=/usr/local/musa/bin/mcc MUSA_ARCH=mp_31 \
timeout 300s target/toolchains/cargo/bin/cargo check \
  --manifest-path rust/Cargo.toml --features musa -j 2
```

Result: exit 0, `Finished dev profile`; the only diagnostic was MUSA clang
raising its requested `-O0` to the compiler-supported optimization level.

The failure path was checked offline after dependency acquisition:

```sh
OMP_NUM_THREADS=2 CARGO_BUILD_JOBS=2 \
CARGO_HOME=/workspace/target/toolchains/cargo \
RUSTUP_HOME=/workspace/target/toolchains/rustup \
CARGO_TARGET_DIR=/workspace/target/rust-mutual \
MCC=/usr/local/musa/bin/mcc MUSA_ARCH=mp_31 \
timeout 180s target/toolchains/cargo/bin/cargo check --offline \
  --manifest-path rust/Cargo.toml --features cuda,musa -j 2
```

Result: expected exit 101 with `cuda, rocm, and musa features are mutually
exclusive` from `rust/build.rs`.

## Remaining gates

- CUDA and ROCm compilers are absent, so their preserved source paths were not
  compile-regressed here: **INCONCLUSIVE**.
- `cargo fmt --check` with Rust 1.97.1 is not clean because the accepted base's
  existing Rust files differ from that formatter's current style; no broad
  formatting rewrite was made for this bounded plumbing change.
- PTX in `ff/{mont_t,mont32_t,gl64_t}.cuh`, `ff/baby_bear.hpp`, MSM, and
  `ec/xyzz_t.hpp` is still NVIDIA-only. Numeric `__CUDA_ARCH__` branches remain
  CUDA-only and were not presented to MUSA.
- NTT/MSM field arithmetic, warp semantics, cooperative-launch capability,
  async allocation behavior, device properties, and numerical correctness are
  deliberately not translated or validated in this stage.
- A later assigned-GPU stage must establish device model/driver, runtime API
  semantics, capability gates, execution, and independent arithmetic oracles.

Nothing in this receipt claims MUSA NTT/MSM support or GPU runtime readiness.
