#!/bin/sh
set -eu

: "${MUSA_ROOT:=/usr/local/musa}"
: "${MUSA_ARCH:=mp_31}"
: "${OUTPUT_DIR:=target/musa-gl64-stage3}"
: "${CXX:=g++}"
MCC=${MCC:-"$MUSA_ROOT/bin/mcc"}

mkdir -p "$OUTPUT_DIR"

"$CXX" -std=c++17 -O2 -I. tests/gl64_portable_test.cpp \
    -o "$OUTPUT_DIR/gl64_portable_test"
"$OUTPUT_DIR/gl64_portable_test"
python3 tests/check_gl64_selection.py

"$MCC" -x musa --offload-arch="$MUSA_ARCH" -I. \
    -include util/cuda2musa.hpp -DFEATURE_GOLDILOCKS \
    -DGL64_NO_REDUCTION_KLUDGE -fPIC -c poc/ntt-cuda/cuda/ntt_api.cu \
    -o "$OUTPUT_DIR/ntt_api.o"
"$MCC" -x musa --offload-arch="$MUSA_ARCH" -I. \
    -include util/cuda2musa.hpp -fPIC -c util/all_gpus.cpp \
    -o "$OUTPUT_DIR/all_gpus.o"
"$MCC" -shared "$OUTPUT_DIR/ntt_api.o" "$OUTPUT_DIR/all_gpus.o" \
    -L"$MUSA_ROOT/lib" -Wl,-rpath,"$MUSA_ROOT/lib" -lmusart \
    -o "$OUTPUT_DIR/libntt_gl64_musa.so"

nm -D --defined-only "$OUTPUT_DIR/libntt_gl64_musa.so" |
    grep -q ' compute_ntt$'
readelf -d "$OUTPUT_DIR/libntt_gl64_musa.so" |
    grep -q 'Shared library: \[libmusart.so.5\]'

"$MCC" -x musa --offload-arch="$MUSA_ARCH" -I. -DFEATURE_GOLDILOCKS \
    -DGL64_NO_REDUCTION_KLUDGE -include util/cuda2musa.hpp -dM -E \
    poc/ntt-cuda/cuda/ntt_api.cu > "$OUTPUT_DIR/musa_macros.txt"
if grep -Eq '__CUDACC__|__NVCC__|__CUDA_ARCH__|__HIPCC__' \
    "$OUTPUT_DIR/musa_macros.txt"; then
    echo "MUSA gl64 compilation exposed a prohibited vendor identity" >&2
    exit 1
fi

echo "MUSA gl64 stage 3 compile/link passed: $OUTPUT_DIR/libntt_gl64_musa.so"
