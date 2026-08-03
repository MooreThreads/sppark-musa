// Copyright Supranational LLC
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#if !defined(__SPPARK_FF_BABY_GOLDILOCKS_HPP__)
#define __SPPARK_FF_BABY_GOLDILOCKS_HPP__

#include <util/gpu_backend.hpp>

#if SPPARK_CUDA_COMPILER
# include "gl64_t.cuh"  // CUDA device-side field types
#elif SPPARK_ROCM_COMPILER
# include "gl64_t.hip"
#elif SPPARK_MUSA_COMPILER
# include "gl64_t.musa"
#endif

namespace goldilocks {
typedef gl64_t fr_t;
}

#ifdef FEATURE_GOLDILOCKS
using namespace goldilocks;
#endif

#endif
