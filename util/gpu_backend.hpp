// Copyright Supranational LLC
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef __SPPARK_UTIL_GPU_BACKEND_HPP__
#define __SPPARK_UTIL_GPU_BACKEND_HPP__

// Compiler identity and device-pass identity are deliberately separate. In
// particular, MUSA is not made to impersonate CUDA: no __CUDACC__, __NVCC__,
// or __CUDA_ARCH__ compatibility definitions belong here.
#if defined(__NVCC__) || defined(__CUDACC__)
# define SPPARK_CUDA_COMPILER 1
#else
# define SPPARK_CUDA_COMPILER 0
#endif

#if defined(__HIPCC__)
# define SPPARK_ROCM_COMPILER 1
#else
# define SPPARK_ROCM_COMPILER 0
#endif

#if defined(__MUSACC__)
# define SPPARK_MUSA_COMPILER 1
#else
# define SPPARK_MUSA_COMPILER 0
#endif

#if SPPARK_CUDA_COMPILER + SPPARK_ROCM_COMPILER + SPPARK_MUSA_COMPILER > 1
# error "SPPARK GPU backends are mutually exclusive"
#endif

#if defined(__CUDA_ARCH__)
# define SPPARK_CUDA_DEVICE 1
#else
# define SPPARK_CUDA_DEVICE 0
#endif

#if defined(__HIP_DEVICE_COMPILE__)
# define SPPARK_ROCM_DEVICE 1
#else
# define SPPARK_ROCM_DEVICE 0
#endif

#if defined(__MUSA_ARCH__)
# define SPPARK_MUSA_DEVICE 1
#else
# define SPPARK_MUSA_DEVICE 0
#endif

#define SPPARK_GPU_COMPILER \
    (SPPARK_CUDA_COMPILER || SPPARK_ROCM_COMPILER || SPPARK_MUSA_COMPILER)
#define SPPARK_GPU_DEVICE \
    (SPPARK_CUDA_DEVICE || SPPARK_ROCM_DEVICE || SPPARK_MUSA_DEVICE)

#endif
