# SPPARK to MUSA 5.2.0 scope and validation receipt

Date: 2026-08-03 (Asia/Shanghai)  
Task: `f1cb46bc-3d09-4fab-aee7-e9086fcde50d`  
Scope: audit and plan only; no production source was converted.

## Decision

**INCONCLUSIVE for MUSA GPU support.** This CPU-only audit establishes a viable
runtime-API mapping and a bounded path to a representative build, but no GPU was
assigned, so device execution and numerical correctness cannot be established.
The unmodified source is **NO_GO for MUSA 5.2.0 compilation**: a bounded `mcc`
probe fails on unmapped `cuda*` names. Nothing here claims GPU support or
performance.

## Frozen source and clean start

- Canonical SSH remote: `git@github.com:supranational/sppark.git`
- Exact upstream commit: `17278d74295392f9813f009300b257a688422b7a`
- Subject/date: `rust/Cargo.toml: bump the version number.`, 2026-06-04
  12:06:56 +0200.
- Initial state: detached `HEAD`; `git status --short` was empty.
- Submodules: none (`git submodule status` was empty); no `.gitmodules` or
  gitlink exists.
- Initial tracked tree: 97 files. `git count-objects -v` reported zero loose and
  garbage objects.

Commands run before any edit:

```sh
pwd
git status --short --branch
git rev-parse HEAD
git remote -v
git show -s --format=fuller HEAD
git submodule status
git ls-tree -r --long HEAD
git ls-files | sort
git count-objects -v
```

Observed identity:

```text
/workspace
## HEAD (no branch)
17278d74295392f9813f009300b257a688422b7a
origin  git@github.com:supranational/sppark.git (fetch)
origin  git@github.com:supranational/sppark.git (push)
```

This receipt is to be committed as a direct child of that commit. No fetch,
install, generated lockfile, translation, or dependency update was performed.

## Repository and build inventory

There is no CMakeLists.txt, Makefile, Bazel file, vendored dependency, or
submodule. Build/integration entry points are:

| Surface | Files and finding |
|---|---|
| Rust package selector | `rust/Cargo.toml`, `rust/build.rs`: finds `nvcc`/`hipcc`, requires CUDA >=11.4 or HIP >=5.7, compiles `util/all_gpus.cpp`, and exports target `cuda` or `rocm`. No MUSA feature or `mcc` detection. |
| Consumer compiler | `rust/src/build.rs`: `cc::Build` config for CUDA (`.cuda(true)`, NVIDIA `-gencode`) or ROCm (`hipcc`, AMD arches, forced `util/cuda2hip.hpp`). No MUSA route. |
| Rust ABI | `rust/src/lib.rs`: errors, GPU pointer/slice, and NTT enums; GPU pointer code is gated only on `cuda`/`rocm`. |
| NTT PoC | `poc/ntt-cuda/{Cargo.toml,build.rs,cuda/ntt_api.cu,src/lib.rs,tests/ntt.rs}`: selects one field and exposes NTT/iNTT/coset calls. |
| MSM PoC | `poc/msm-cuda/{Cargo.toml,build.rs,cuda/*.cu,src/*.rs,tests/msm.rs,benches/msm.rs}`: BLST host objects plus Pippenger CUDA; Arkworks oracle. |
| Go builder/loader | `go/sppark.go`, `go/cgo_sppark.h`, `go/README.md`: locates only `nvcc`/`hipcc`, builds BLST objects and a shared object, then `dlopen`/`LoadLibrary`. |
| Go PoCs | `poc/go/*`, `poc/ntt-cuda/go/*`: minimal kernel/load and Goldilocks round trip. |
| CI | `.github/workflows/ci.yml`: Ubuntu 22.04 compile-only CUDA 12.9/11.8 coverage. No GPU execution, ROCm, or MUSA job. |

README support is x86_64 NVIDIA Volta+ on Linux/Windows, limited AMD RDNA/CDNA,
and host-only ARM64/macOS. The first MUSA target is **Linux x86_64**, the only
platform evidenced here. Windows, ARM64, and other MUSA platforms are deferred.

### Exhaustive tracked-file classification

Every tracked file is covered by one group:

- GPU translation units/include fragments: `ntt/kernels.cu`, all four
  `ntt/kernels/*.cu`, `poc/go/poc.cu`, `poc/msm-cuda/cuda/*.cu`,
  `poc/ntt-cuda/cuda/ntt_api.cu`, `util/cuda_available.cu`.
- Mixed device/host templates: all `ec/*.hpp`, all `ff/*.hpp`, `ff/*.cuh`, all
  `msm/*.cuh`, `ntt/{ntt.cuh,parameters.cuh}`, all `polynomial/*.cuh`, and
  `util/{gpu_t.cuh,cuda_timer_t.cuh,exception.cuh,slice_t.hpp,vec2d_t.hpp}`.
  `.hpp` therefore does not imply host-only.
- ROCm-only arithmetic/compatibility: `ff/*.hip`, `util/cuda2hip.hpp`. AMD GCN
  assembly cannot serve as MUSA device code.
- Host-only C/C++: `msm/pippenger.hpp`,
  `util/{exception.hpp,rusterror.h,thread_pool_t.hpp}`, plus host runtime
  orchestration `util/all_gpus.cpp` and CUDA host probe
  `rust/src/cuda_available.cpp` (both still GPU-compiler/API dependent).
- Static NTT parameter data: `ntt/parameters/*.h` and its `.gitattributes`.
- Host bindings/manifests/tests/benchmarks/scripts: all tracked `.rs`, `.go`,
  `.mod`, `.sum`, `.c`, and bridge `.h` files under `rust/`, `go/`, and `poc/`
  except GPU units above; also `rust/publish.sh`, Cargo manifests/build scripts,
  and `poc/msm-cuda/rustfmt.toml`.
- Metadata/docs: `.github/workflows/ci.yml`, `.gitignore`, `LICENSE`,
  `README.md`, `go/README.md`.

README directories `conversion`, `hash`, `memory`, and `merkle` do not exist at
this commit and are not in scope.

## CUDA language and device-code surface

Inventory commands:

```sh
rg -l '__global__|<<<|cuda[A-Z]|__device__|__CUDACC__|__CUDA_ARCH__' \
  --glob '*.{h,hpp,cuh,cu,cpp}' . | sort
rg -n 'asm\s*\(|__CUDA_ARCH__|__NVCC__|warpSize|WARP_SZ|__shfl|__ballot|\
__activemask|__syncwarp|__syncthreads|atomic(Add|Sub)' \
  ec ff msm ntt polynomial util poc --glob '*.{h,hpp,cuh,cu,hip,cpp}'
```

| Construct | File-level evidence |
|---|---|
| Kernels/launches/shared memory | `msm/{batch_addition,pippenger,sort}.cuh`; `ntt/kernels.cu`; `ntt/kernels/*.cu`; `ntt/{ntt,parameters}.cuh`; all `polynomial/*.cuh`; `poc/go/poc.cu` |
| Device/host functions/constants | all `ec/*.hpp`; field headers and `ff/*.cuh`; `ntt/parameters.cuh`; `util/{slice_t,vec2d_t}.hpp` |
| Cooperative grid sync | all `msm/*.cuh`, `ntt/kernels.cu`, all `polynomial/*.cuh`; launched through `util/gpu_t.cuh` |
| Warp intrinsics/fixed 32 lanes | field arithmetic/shuffle headers, all `msm/*.cuh`, NTT kernels, `util/cuda2hip.hpp`; repeated `WARP_SZ 32` and mask `0xffffffff` |
| Atomics/barriers | MSM, NTT, polynomial kernels: `atomicAdd/Sub`, `__syncthreads`, `__syncwarp` |
| Compiler/arch gates | `__NVCC__`, `__CUDACC__`, `__CUDA_ARCH__` across field type selection, NTT paths, `util/gpu_t.cuh`, `util/all_gpus.cpp`, availability probes |
| CUDA PTX assembly | Dense in `ff/{mont_t,mont32_t,gl64_t}.cuh`; also `ff/baby_bear.hpp`, all `msm/*.cuh`, `ec/xyzz_t.hpp` (`prefetch.global.L2`) |
| AMD GCN assembly | `ff/{mont_t,mont32_t,gl64_t}.hip`, ROCm helpers in `util/cuda2hip.hpp` |

MUSA `mcc` defines `__MUSACC__`, `__MUSA__`, `__MUSA_ARCH__`, and `__CUDA__`,
but the probe did not show `__NVCC__`, `__CUDACC__`, or `__CUDA_ARCH__`.
Current branches will therefore choose wrong types or `#error` even after API
mapping. Globally defining CUDA macros is unsafe because numeric architecture
tests and PTX are NVIDIA-specific.

MUSA 5.2 headers provide CUDA-like kernel syntax, built-ins, atomics, barriers,
shuffle/ballot/active-mask intrinsics, and `cooperative_groups.h`. Presence is
not semantic validation: 32-lane masks, grid sync/residency, dynamic shared
memory, async allocation ordering, and arithmetic carry/borrow remain blockers.

## CUDA API/library inventory and MUSA disposition

There is no cuBLAS, cuFFT, CUB, Thrust, NCCL, NVRTC, or other CUDA library use.
Only CUDA runtime/driver-facing headers and runtime are required. `<cuda.h>` is
included by MSM entry points; `<cuda_runtime.h>` by `util/gpu_t.cuh` and the Rust
probe. Runtime use is mostly centralized in `util/gpu_t.cuh`.

A mechanical search found same-suffix `musa*` identifiers in installed MUSA
5.2.0 headers for all 57 repository CUDA runtime identifiers; none was missing.
Presence does not prove behavior.

| Group | CUDA identifiers | Disposition |
|---|---|---|
| Device/error | `cudaDeviceProp`, `cudaError_t`, `cudaSuccess`, `cudaErrorNoDevice`, `cudaGetDeviceCount`, `cudaGet/SetDevice`, `cudaGetDeviceProperties`, `cudaDeviceSynchronize`, `cudaGetLastError`, `cudaGetErrorString`, `cudaMemGetInfo` | Direct `musa*` counterparts present; central mapping plus property semantics validation. |
| Streams/events | `cudaStream_t`, create/flags/destroy/sync/wait; `cudaEvent_t`, create/flags/record/sync/elapsed/destroy | Direct counterparts present. |
| Allocation/mapping | `cudaMalloc`, `cudaMallocAsync`, `cudaMallocManaged`, `cudaFree`, `cudaFreeAsync`, host alloc/malloc/free, four host flags, `cudaHostGetDevicePointer`, `cudaHostGetFlags` | Direct counterparts present; async pool is device-dependent. |
| Copy/set | `cudaMemcpyKind`, three directions, `cudaMemcpy`, `cudaMemcpyAsync`, `cudaMemcpy2DAsync`, `cudaMemsetAsync` | Direct counterparts present. |
| Launch/function | `cudaLaunchCooperativeKernel`, `cudaLaunchHostFunc`, `cudaHostFn_t`, `cudaFuncAttributes`, get/set/maximum-dynamic-shared attribute, occupancy query, symbol address | Direct counterparts present; capability/semantics require GPU validation. |
| Local ABI names | `CUDA_OK`, exported `cuda_available`, Rust `cuda_error!`, `Gpu_Ptr` | Project-owned; preserve external ABI if useful, neutralize internals. |

Exact presence-check shape:

```sh
for api in $(rg -o --no-filename '\bcuda[A-Z][A-Za-z0-9_]*\b' \
  --glob '*.{h,hpp,cuh,cu,cpp}' . | sort -u); do
  musa="musa${api#cuda}"
  rg -q "\b${musa}\b" /usr/local/musa/include --glob '*.{h,hpp}' \
    && printf 'FOUND %s -> %s\n' "$api" "$musa" \
    || printf 'MISSING %s -> %s\n' "$api" "$musa"
done
```

## Complete dependency disposition

"Reusable" means no vendor coupling was found in its use here, not that it was
installed or built.

| Dependency | MUSA 5.2.0 disposition |
|---|---|
| `mcc`, headers, `libmusart` | **Present**, exact 5.2.0. Add explicit `musa` backend and link via `mcc`; `/usr/local/musa/bin` is outside `PATH`. |
| CUDA >=11.4/runtime | Existing optional NVIDIA backend; retain, not needed for MUSA. `nvcc` absent. |
| HIP >=5.7/`libamdhip64` | Existing optional AMD backend; retain, not needed for MUSA. `hipcc` absent. HIP aliases/GCN assembly are not MUSA code. |
| C++17/STL/pthreads | GCC/G++ 11.4 present; host header probes pass. `mcc` embeds Clang 14. Full host/device TU compatibility unresolved. |
| Rust/Cargo | Vendor-neutral and required for representative build/tests; **absent**, so local integration is INCONCLUSIVE. |
| Rust `cc ^1.0.70`, `which ^4.0` | Host crates. `which` can locate `mcc`; `.cuda(true)` is CUDA-specific, so add verified MUSA command/backend logic. Not fetched. |
| BLST (`~0.3.11` Rust, `v0.3.16` Go, `blst_t.hpp`) | Reusable host field/curve and MSM assembly dependency. Required for MSM/large-field NTT; not fetched/built. Host ISA remains separate concern. |
| Semolina `~0.1.2`/`pasta_t.hpp` | Reusable optional host types; not needed for first build; not fetched. |
| Arkworks 0.3 family | Reusable host-only independent oracle; essential for later correctness; not fetched. |
| `rand`, `rand_core`, `rand_chacha` | Reusable host-only test vector dependencies; not fetched. |
| Criterion 0.3 | Host benchmark only; out of initial gate. |
| Go 1.18+, cgo, C compiler, `libdl` | Host tooling; add `mcc` branch/runtime linkage. Go absent. Linux `libdl` first; Windows deferred. |
| `cooperative_groups.h` | MUSA header present; target device capability/behavior unresolved and mandatory. |
| Device intrinsics/assembly | Common intrinsics present. PTX/GCN assembly is nonportable; reviewed MUSA backend or portable arithmetic is the largest blocker. |
| GitHub Actions/cache, wget/apt, sccache | CI conveniences, not runtime dependencies. Future MUSA CI should be pre-provisioned; no install/fetch in this task. |

No hidden submodules exist. Cargo and Go manifests above contain the complete
declared external set.

## Bounded non-GPU probes

All used `OMP_NUM_THREADS=2`; timeouts bounded compilations. No GPU enumeration
or execution was run. The host reports 128 CPUs, but only two were assigned.

```sh
export OMP_NUM_THREADS=2
uname -a
getconf _NPROCESSORS_ONLN
command -v mcc nvcc hipcc cargo rustc go gcc g++ clang++ cmake
/usr/local/musa/bin/mcc --version
/usr/local/musa/bin/musa_toolkits_version
```

Result: Linux x86_64; GCC/G++ 11.4 and CMake present; GPU compilers, Rust, Go,
and Clang absent from `PATH`. Exact-path tools report:

```text
clang version 14.0.0
mcc version 5.2.0
Target: x86_64-unknown-linux-gnu
musa_toolkits version: 5.2.0
toolkit tag: 20260602_master
toolkit commit: 8c44cbed02a79be2066f1f62bdca039177f71d51
```

Host syntax probes:

```sh
timeout 60s g++ -std=c++17 -I. -x c++ -fsyntax-only \
  -include util/thread_pool_t.hpp /dev/null
timeout 60s g++ -std=c++17 -I. -x c++ -fsyntax-only \
  -include msm/pippenger.hpp /dev/null
```

Both exit 0. They prove parsing only, not algorithms.

Current-source MUSA probe:

```sh
timeout 60s /usr/local/musa/bin/mcc -x musa -fsyntax-only poc/go/poc.cu
```

Exit 1: five host-side errors for `cudaGetLastError`, `cudaSuccess`,
`cudaGetErrorString` (twice), and `cudaDeviceSynchronize`; `mcc` suggested each
`musa*` counterpart. This proves a compatibility/build layer is required, not
anything about the deeper PTX surface.

Skipped explicitly:

- Cargo/Go config/build: toolchains absent and fetch forbidden: **INCONCLUSIVE**.
- Full NTT/MSM MUSA build: conversion is out of scope and current source already
  fails: **NO_GO current source**, future build unresolved.
- GPU runtime/correctness/performance: no GPU assigned: **INCONCLUSIVE**.
- CUDA/ROCm regression: compilers absent: **INCONCLUSIVE**.

## Denominators and staged port plan

### Representative build denominator

Use `poc/ntt-cuda` feature `gl64`, release, Linux x86_64, MUSA 5.2.0, one frozen
MUSA architecture, and at most two jobs. It exercises real NTT runtime,
twiddles, narrow/wide kernels, launches, and Goldilocks arithmetic while avoiding
BLST/semolina device coupling and MSM. `poc/go/poc.cu` is only an earlier
plumbing smoke test.

Future command shape after a MUSA Cargo backend exists:

```sh
export OMP_NUM_THREADS=2 CARGO_BUILD_JOBS=2
export MCC=/usr/local/musa/bin/mcc
timeout 20m cargo test --manifest-path poc/ntt-cuda/Cargo.toml \
  --no-run --release --features=gl64,musa
```

Record an explicit `--offload-arch`; `native` is not reproducible. The exact
architecture is blocked on assignment of a target MUSA GPU.

### Independent GPU correctness denominator

Use `poc/ntt-cuda` feature `bls12_381` and existing
`test_against_arkworks`: it independently checks forward, inverse, coset, and
NR/RN ordering. Begin with its debug-size range, device 0, one assigned MUSA
GPU, and two host threads. This is the smallest existing independent oracle for
a common proof field. Goldilocks self-consistency is useful but cannot catch a
matched forward/inverse defect. A success report must include device model,
driver/runtime, architecture flag, exact command, and all assertions.

### Stages and exit criteria

1. **Backend plumbing:** add mutually exclusive `musa` Rust/Go discovery,
   target cfg, ABI guards, headers, runtime link, and availability path. Central
   API mapping only. Exit: minimal Go kernel and `util/all_gpus.cpp` compile/link
   without executing.
2. **Macro normalization:** replace semantic `__NVCC__`, `__CUDACC__`, and
   `__CUDA_ARCH__` uses with project backend/host/device macros; preserve CUDA
   and ROCm. Exit: compile-only selection tests for all backends.
3. **Goldilocks arithmetic:** add reviewed MUSA-specific or portable
   carry/borrow implementation; never feed PTX/GCN assembly to `mcc`. Add host
   bit-vector/layout tests. Exit: selected `gl64` release denominator compiles.
4. **Runtime/NTT:** map streams, async memory, events, cooperative launch,
   attributes/properties/symbols; reject missing required capabilities. Exit:
   compile/link and GPU enumeration on a later assigned runner, without yet
   claiming correctness.
5. **GPU correctness:** deterministic small Goldilocks round trips, then
   BLS12-381 versus Arkworks across all existing cases. Any failure is NO_GO.
6. **MSM expansion:** port multi-limb/FP2 and Pippenger/sort/batch addition;
   audit all 32-lane/assembly assumptions; run G1/G2 Arkworks tests at bounded
   `TEST_NPOW`, then scale.
7. **Regression/CI:** retain CUDA/ROCm compilation, add pre-provisioned MUSA 5.2
   compile and GPU correctness jobs, and missing-tool/no-device cases before
   any support documentation or benchmark claim.

## Risks and blockers

1. PTX implements critical arithmetic/scheduling, not optional tuning; blind
   translation is unsafe.
2. MUSA compiler macros differ and CUDA numeric architecture tests are invalid.
3. Full-mask shuffle and fixed 32-lane semantics need target-device proof.
4. Cooperative grid synchronization is mandatory; capability/occupancy limits
   require a target GPU.
5. Async allocation behavior is device-dependent and used normally.
6. Rust/Go integration cannot be built locally without forbidden installation.
7. No target MUSA GPU/architecture is specified, preventing a durable binary
   target or runtime conclusion.
8. Existing upstream CI is compile-only and supplies no GPU baseline.

The next task should start with stages 1-2 and the minimal smoke compile, not a
repository-wide translation.
