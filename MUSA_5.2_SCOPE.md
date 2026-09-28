# SPPARK MUSA support scope

This repository is the Moore Threads MUSA maintenance line of SPPARK v0.1.15.
The source retains the upstream finite-field, elliptic-curve, MSM, NTT, CUDA,
ROCm, Rust, and C++ structure. The MUSA work is organized as an additional
backend path and does not redefine the upstream APIs.

The MUSA SDK and compiler are external prerequisites. Backend availability,
target architecture, and application-specific numerical validation must be
checked by each consuming deployment; this note makes no performance or
runtime qualification claim.
