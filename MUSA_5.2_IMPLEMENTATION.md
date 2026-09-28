# SPPARK MUSA implementation note

The Moore Threads port adds MUSA-aware compiler and backend plumbing while
preserving the upstream SPPARK directory layout and public interfaces. Build
selection is explicit: configure the MUSA compiler, target architecture, and
runtime together with the existing Rust and C++ options, or select the CUDA or
ROCm path when those backends are required.

The implementation remains source-compatible with the inherited project
organization. Refer to `README.md` for the public build entry points; optional
toolkit components are supplied by the target environment rather than bundled
by this repository.
