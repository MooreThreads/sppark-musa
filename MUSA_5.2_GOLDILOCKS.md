# MUSA 5.2 Goldilocks compile-frontier receipt

Date: 2026-08-04 (Asia/Shanghai)

Task: `b30eb7d3-6891-449b-b182-eb47df86e3e5`

Base: exact accepted S1 result `95e27ad36588e67986a8e9b51d180a6452ab719a`

## Result and boundary

**INCONCLUSIVE for correctness and GPU support. PASS for the bounded stage 3
host arithmetic/selection tests and authentic Goldilocks NTT compile/link
fallback.** No GPU was assigned, enumerated, or used. No shared object or kernel
was loaded or executed. CUDA and ROCm source paths remain present but their
compilers were unavailable, so their compilation remains inconclusive.

The exact Cargo release denominator could not be started because the fresh
workspace had no Rust toolchain and two bounded official rustup attempts timed
out downloading channel or standard-library data. Per the frozen plan, the
fallback compiled and linked `poc/ntt-cuda/cuda/ntt_api.cu`, the smallest
authentic production translation unit for the same gl64 NTT path. It includes
the real NTT parameters, twiddle generation, narrow/wide kernels, kernel
launches, and `compute_ntt`; it is not a runtime-only synthetic kernel.

## Bounded changes

- `ff/gl64_t.musa` supplies a MUSA-only portable `gl64_t` with C++ carry,
  borrow, multiplication, reduction, selection, shifts, exponentiation, and
  shuffle. It contains no PTX, AMD GCN assembly, fake NVIDIA identity, or CUDA
  numeric architecture gate.
- The 128-bit product is reduced using the Goldilocks identity
  `2^64 = 2^32 - 1 (mod 0xffffffff00000001)`. An initial `% MOD` lowering
  caused the MUSA LLVM instruction selector to crash in
  `generate_inner_twiddles<gl64_t>`; the reviewed limb reduction avoids
  device-side 128-bit division and compiles successfully.
- `ff/goldilocks.hpp` selects `.cuh` for CUDA, `.hip` for ROCm, and `.musa` for
  MUSA through centralized S1 backend identity. Immediate host/device gates
  reached by this NTT path now recognize MUSA without defining `__CUDACC__`,
  `__NVCC__`, or `__CUDA_ARCH__`.
- `poc/ntt-cuda` forwards its explicit `musa` feature to `sppark/musa`; the
  existing CUDA and ROCm build branches are unchanged.
- Host tests cover 64-bit size/alignment/layout, fixed boundary vectors, 4096
  deterministic carry/borrow/multiply vectors against an independent
  `unsigned __int128` remainder reference, halving/doubling, inverse, and
  seventh-root identities. The selection test applies in-memory mutations and
  proves it fails if MUSA selects CUDA/PTX or ROCm/GCN, or gains fake NVIDIA or
  CUDA architecture tokens.

## Tool, dependency, and resource identity

All build commands used `OMP_NUM_THREADS=2`, and no command exceeded the
assigned two CPUs. Architecture was explicitly `mp_31`.

```text
clang version 14.0.0
mcc version 5.2.0
Target: x86_64-unknown-linux-gnu
musa_toolkits version: 5.2.0
toolkit tag: 20260602_master
toolkit commit: 8c44cbed02a79be2066f1f62bdca039177f71d51
```

The official rustup shell bootstrap SHA-256 was
`6c30b75a75b28a96fd913a037c8581b580080b6ee9b8169a3c0feb1af7fe8caf`.
Because `/tmp` is mounted no-exec, the official x86_64 rustup-init binary was
downloaded directly under the ignored retained tool root; its SHA-256 was
`4acc9acc76d5079515b46346a485974457b5a79893cfb01112423c89aeb5aa10`.
The selected stable identity was Rust/Cargo 1.97.1 dated 2026-07-16, but the
installation remained incomplete after network timeouts. No host install or
package manager was used.

## Exact successful verification

From repository root:

```sh
chmod +x tests/check_musa_gl64_stage3.sh tests/check_gl64_selection.py
OUTPUT_DIR=/workspace/target/stage3-evidence-2 \
OMP_NUM_THREADS=2 MCC=/usr/local/musa/bin/mcc MUSA_ARCH=mp_31 \
timeout 600s tests/check_musa_gl64_stage3.sh
```

Output:

```text
MUSA Goldilocks selection and mutation checks passed
MUSA gl64 stage 3 compile/link passed: /workspace/target/stage3-evidence-2/libntt_gl64_musa.so
```

The script runs the host executable, mutation checker, and these production
compile/link operations without loading the result:

```sh
g++ -std=c++17 -O2 -I. tests/gl64_portable_test.cpp -o "$OUTPUT_DIR/gl64_portable_test"
"$OUTPUT_DIR/gl64_portable_test"
python3 tests/check_gl64_selection.py
/usr/local/musa/bin/mcc -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -DFEATURE_GOLDILOCKS \
  -DGL64_NO_REDUCTION_KLUDGE -fPIC -c poc/ntt-cuda/cuda/ntt_api.cu \
  -o "$OUTPUT_DIR/ntt_api.o"
/usr/local/musa/bin/mcc -x musa --offload-arch=mp_31 -I. \
  -include util/cuda2musa.hpp -fPIC -c util/all_gpus.cpp \
  -o "$OUTPUT_DIR/all_gpus.o"
/usr/local/musa/bin/mcc -shared "$OUTPUT_DIR/ntt_api.o" \
  "$OUTPUT_DIR/all_gpus.o" -L/usr/local/musa/lib \
  -Wl,-rpath,/usr/local/musa/lib -lmusart \
  -o "$OUTPUT_DIR/libntt_gl64_musa.so"
```

Object/link inspection:

```text
ntt_api.o: ELF 64-bit LSB relocatable, x86-64, not stripped
libntt_gl64_musa.so: ELF 64-bit LSB shared object, x86-64, dynamically linked
000000000000ecd0 T compute_ntt
0000000000014900 T cuda_available
Shared library: [libmusart.so.5]
Library runpath: [/usr/local/musa/lib]
```

The script also records MUSA preprocessor macros and fails on any of
`__CUDACC__`, `__NVCC__`, `__CUDA_ARCH__`, or `__HIPCC__`.

## Exact Cargo blocker

Intended denominator:

```sh
OMP_NUM_THREADS=2 CARGO_BUILD_JOBS=2 \
CARGO_HOME=/workspace/target/toolchains/cargo-stage3 \
RUSTUP_HOME=/workspace/target/toolchains/rustup-stage3 \
CARGO_TARGET_DIR=/workspace/target/stage3-cargo \
MCC=/usr/local/musa/bin/mcc MUSA_ARCH=mp_31 \
timeout 1200s /workspace/target/toolchains/cargo-stage3/bin/cargo test \
  --manifest-path poc/ntt-cuda/Cargo.toml --no-run --release \
  --features=gl64,musa -j 2
```

The first attempt could not execute Cargo because it was absent. Official
rustup acquisition then failed first on `rust-std-1.97.1` with TCP timeout and
again on `channel-rust-stable.toml.sha256` with TCP timeout. No Cargo build was
reported as run or passed.

## Remaining frontier

The next task is frozen stage 4 on an assigned MUSA GPU: establish device and
driver identity, validate warp width/masks, runtime properties, stream/event
and async-allocation ordering, symbol handling, occupancy, dynamic shared
memory, and cooperative-launch capability/residency before enumeration or NTT
execution. Only after those gates may deterministic Goldilocks execution and
an independent BLS12-381 Arkworks oracle be attempted. This receipt makes no
runtime, numerical-correctness, performance, CUDA-regression, or ROCm-regression
claim.
