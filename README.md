# sppark-musa

`sppark-musa` is the Moore Threads MUSA-maintained fork of
[supranational/sppark](https://github.com/supranational/sppark), a collection
of C++ templates and language bindings for high-performance operations used by
zero-knowledge proof systems. The library contains reusable finite-field,
elliptic-curve, multi-scalar multiplication (MSM), and number-theoretic
transform (NTT) building blocks.

This fork keeps the upstream CUDA and ROCm implementations and adds a MUSA
backend for the supported integration paths. Moore Threads maintains the fork
so applications can adapt the same primitives to MUSA toolchains while
retaining the upstream source organization and APIs.

## Source lineage

- Upstream project: `https://github.com/supranational/sppark`
- Inherited upstream version: `v0.1.15`
- Upstream baseline: `17278d74295392f9813f009300b257a688422b7a`
- MUSA adaptation ref: `sppark-musa`

The upstream implementation remains the reference for the CUDA and ROCm
paths. Changes in this fork are isolated around compiler selection, MUSA API
compatibility, and the MUSA field implementation.

## MUSA adaptation

The MUSA path is selected explicitly in the Rust and Go integration layers:

- Rust exposes the `musa` feature and uses `MCC` and `MUSA_ARCH` to select the
  MUSA compiler and device target. The default device target is `mp_31`.
- The Go bridge accepts `SPPARK_BACKEND=musa` and `MCC`, and links the MUSA
  runtime for Linux builds.
- `util/cuda2musa.hpp` provides the MUSA spelling boundary for the APIs used
  by the port, while `util/gpu_backend.hpp` centralizes backend identity.
- `ff/gl64_t.musa` supplies the MUSA implementation of the Goldilocks field;
  the existing CUDA and ROCm field sources remain available for their
  respective backends.
- The NTT Cargo package forwards its `musa` feature to the core `sppark`
  crate. Existing CUDA and ROCm feature paths are retained.

The MUSA integration is intended for Linux systems with a compatible Moore
Threads toolchain. Its source-level scope is deliberately bounded; consumers
should select and review the field and operation paths they use.

## Requirements

For the MUSA path, install:

- a MUSA toolkit providing `mcc` and the `musart` runtime;
- Rust and Cargo for the Rust crates;
- a C/C++ toolchain and CMake-free Cargo build environment;
- Go and cgo when using the Go bridge or Go proof-of-concept programs.

The CUDA and ROCm paths continue to use their corresponding upstream
toolchains. Rust and Go dependencies are declared in the package manifests and
are resolved by their normal package managers.

## Building

From the repository root, select the MUSA compiler and target before building
the Rust core crate:

```sh
export MCC=/path/to/musa/bin/mcc
export MUSA_ARCH=mp_31
cargo build --manifest-path rust/Cargo.toml --features musa
```

To build the NTT proof-of-concept crate for the Goldilocks field:

```sh
cargo build --manifest-path poc/ntt-cuda/Cargo.toml \
  --features 'musa,gl64'
```

The Go bridge selects the backend through the environment and can be built from
its module directory:

```sh
cd poc/go
export SPPARK_BACKEND=musa
export MCC=/path/to/musa/bin/mcc
go test ./...
```

Use `MUSA_ARCH` to target a supported MUSA architecture. The CUDA and ROCm
builds continue to use their existing upstream feature and compiler settings;
do not combine more than one of `cuda`, `rocm`, and `musa` in a single Rust
build.

## Using the library

The Rust crate exposes the common error, device-pointer, and NTT interfaces
used by the proof-of-concept packages. The NTT package provides forward,
inverse, and coset transforms through its Rust API. The Go package loads a
shared object next to the calling executable and exposes wrappers through
cgo; see [`go/README.md`](go/README.md) and the examples under `poc/` for the
integration shape.

Applications should choose a field feature supported by their operation and
pass the device ordinal expected by their deployment. Backend selection is
explicit when multiple GPU toolchains are installed.

## Repository layout

- `ec/`, `ff/`: elliptic-curve and finite-field templates;
- `msm/`: multi-scalar multiplication templates;
- `ntt/`: NTT kernels and parameter data;
- `polynomial/`: polynomial helper operations;
- `util/`: backend, device, error, and utility helpers;
- `rust/`: the Rust crate and compiler integration;
- `go/`: the reusable Go bridge;
- `poc/`: Rust and Go proof-of-concept integrations;
- `tests/`: source-level and backend-selection checks.

## Contributing

Keep backend-specific changes behind the corresponding feature or backend
guard, preserve the CUDA and ROCm paths, and document any MUSA compiler or
architecture assumptions. Changes should include focused source-level coverage
when they alter backend selection or public interfaces. Please retain upstream
copyright and attribution notices in modified files.

## Attribution and license

This project retains the upstream sppark structure and attribution. The Moore
Threads fork is distributed under the Apache 2.0 terms in the root
[`LICENSE`](LICENSE) file; component notices in the source tree remain part of
the corresponding components.
