#!/bin/sh
# Compile/link only. This script must never execute the resulting shared object.
set -eu

: "${MUSA_ROOT:=/usr/local/musa}"
: "${MUSA_ARCH:=mp_31}"
: "${OMP_NUM_THREADS:=2}"
: "${OUTPUT_DIR:?set OUTPUT_DIR to a writable evidence directory}"

MCC=${MCC:-"$MUSA_ROOT/bin/mcc"}
mkdir -p "$OUTPUT_DIR"

if printf '\n' | g++ -I. -D__MUSACC__ -D__HIPCC__ \
    -include util/gpu_backend.hpp -x c++ -fsyntax-only - 2>/dev/null; then
    echo "backend mutual-exclusion check unexpectedly compiled" >&2
    exit 1
fi

printf '\n' | g++ -I. -D__MUSACC__ -include util/gpu_backend.hpp \
    -x c++ -dM -E - | grep -q '^#define SPPARK_MUSA_COMPILER 1$'

"$MCC" -x musa --offload-arch="$MUSA_ARCH" -I. \
    -include util/cuda2musa.hpp -fPIC -c poc/go/poc.cu \
    -o "$OUTPUT_DIR/poc.o"
"$MCC" -x musa --offload-arch="$MUSA_ARCH" -I. \
    -include util/cuda2musa.hpp -fPIC -c util/all_gpus.cpp \
    -o "$OUTPUT_DIR/all_gpus.o"
"$MCC" -shared "$OUTPUT_DIR/poc.o" "$OUTPUT_DIR/all_gpus.o" \
    -L"$MUSA_ROOT/lib" -Wl,-rpath,"$MUSA_ROOT/lib" -lmusart \
    -o "$OUTPUT_DIR/poc_musa.so"

readelf -d "$OUTPUT_DIR/poc_musa.so" |
    grep -q 'Shared library: \[libmusart.so.5\]'

if "$MCC" -x musa --offload-arch="$MUSA_ARCH" -I. -dM -E \
    poc/go/poc.cu | grep -Eq '^#define (__CUDACC__|__NVCC__|__CUDA_ARCH__)'; then
    echo "MUSA compilation exposed a fake NVIDIA compiler/architecture macro" >&2
    exit 1
fi

echo "MUSA compile/link frontier passed: $OUTPUT_DIR/poc_musa.so"
