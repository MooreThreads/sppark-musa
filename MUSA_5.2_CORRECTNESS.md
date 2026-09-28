# SPPARK MUSA correctness scope

This note records the public scope of the Moore Threads MUSA adaptation. The
MUSA path is maintained alongside the inherited CUDA and ROCm implementations,
with the upstream finite-field, NTT, MSM, Rust, and C++ organization retained.

The adapter is intended to keep backend selection explicit and to preserve the
upstream APIs. Applications should select the MUSA compiler, architecture, and
runtime through their normal build configuration. This note describes the
supported architecture boundary; it is not a performance claim or a substitute
for project-specific validation in a downstream environment.

See `README.md` for the public build overview and `LICENSE` for the project
license declaration.
