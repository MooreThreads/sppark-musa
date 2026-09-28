# SPPARK MUSA Goldilocks integration note

The MUSA adaptation retains SPPARK's Goldilocks and NTT source organization,
including the existing CUDA-facing interfaces and the optional Rust bindings.
MUSA compiler and architecture selection is kept separate from CUDA and ROCm
selection so that inherited backends remain available.

The exact transform parameters, application inputs, and build configuration
are selected by the consuming project. This public note intentionally does not
publish internal work records, device inventories, or runtime results.
