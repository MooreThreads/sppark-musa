#!/usr/bin/env python3
"""Selection and in-memory mutation checks for the MUSA Goldilocks path."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SELECTOR = ROOT / "ff/goldilocks.hpp"
MUSA_IMPL = ROOT / "ff/gl64_t.musa"


def validate(selector: str, implementation: str) -> None:
    branches = dict(re.findall(
        r"#(?:if|elif)\s+(SPPARK_(?:CUDA|ROCM|MUSA)_COMPILER)\s*\n"
        r"# include \"([^\"]+)\"", selector
    ))
    expected = {
        "SPPARK_CUDA_COMPILER": "gl64_t.cuh",
        "SPPARK_ROCM_COMPILER": "gl64_t.hip",
        "SPPARK_MUSA_COMPILER": "gl64_t.musa",
    }
    if branches != expected:
        raise AssertionError(f"Goldilocks backend selection changed: {branches!r}")

    prohibited = ("asm(", "__asm__", "__CUDACC__", "__NVCC__",
                  "__CUDA_ARCH__", "__HIPCC__", "__AMDGCN",
                  "gl64_t.cuh", "gl64_t.hip")
    found = [token for token in prohibited if token in implementation]
    if found:
        raise AssertionError(f"prohibited backend identity/assembly in MUSA: {found}")


selector = SELECTOR.read_text(encoding="utf-8")
implementation = MUSA_IMPL.read_text(encoding="utf-8")
validate(selector, implementation)

# Each mutation aliases MUSA to a vendor path and must be rejected.
for bad_path in ("gl64_t.cuh", "gl64_t.hip"):
    mutant = selector.replace(
        '# include "gl64_t.musa"', f'# include "{bad_path}"', 1
    )
    try:
        validate(mutant, implementation)
    except AssertionError:
        pass
    else:
        raise AssertionError(f"selection mutation unexpectedly survived: {bad_path}")

# A fake NVIDIA compiler or numeric CUDA architecture gate in the MUSA
# implementation must likewise make the checker fail.
for token in ("__CUDACC__", "__NVCC__", "__CUDA_ARCH__", "asm("):
    try:
        validate(selector, implementation + "\n" + token)
    except AssertionError:
        pass
    else:
        raise AssertionError(f"prohibited-token mutation unexpectedly survived: {token}")

print("MUSA Goldilocks selection and mutation checks passed")
