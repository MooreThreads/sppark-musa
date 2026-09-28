# SPPARK MUSA support and maintenance

SPPARK MUSA is maintained against the upstream v0.1.15 source line. Moore
Threads maintains the MUSA backend and its build integration while retaining
the upstream CUDA and ROCm implementations, source layout, and public APIs.

For support, document the selected backend, compiler, target architecture, and
external toolkit versions in the consuming project's build records. Keep
changes scoped to the relevant backend and preserve upstream attribution and
component notices. This document is a public maintenance guide, not a test
report.
