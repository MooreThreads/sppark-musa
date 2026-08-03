// Copyright Supranational LLC
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef __SPPARK_UTIL_CUDA2MUSA_HPP__
#define __SPPARK_UTIL_CUDA2MUSA_HPP__

#include "gpu_backend.hpp"

#if !SPPARK_MUSA_COMPILER
# error "cuda2musa.hpp requires the MUSA compiler"
#endif

#include <musa_runtime.h>

// This boundary intentionally contains only the CUDA-runtime spellings used
// by poc/go/poc.cu and util/all_gpus.cpp (including gpu_t.cuh). It is not an
// arithmetic or device-intrinsic compatibility layer.
#define cudaDeviceProp                         musaDeviceProp
#define cudaGetDeviceCount                     musaGetDeviceCount
#define cudaGetDevice                          musaGetDevice
#define cudaSetDevice                          musaSetDevice
#define cudaGetDeviceProperties                musaGetDeviceProperties
#define cudaDeviceSynchronize                  musaDeviceSynchronize
#define cudaMemGetInfo                         musaMemGetInfo

#define cudaError_t                            musaError_t
#define cudaGetLastError                       musaGetLastError
#define cudaGetErrorString                     musaGetErrorString
#define cudaSuccess                            musaSuccess
#define cudaErrorNoDevice                      musaErrorNoDevice

#define cudaEvent_t                            musaEvent_t
#define cudaEventCreateWithFlags               musaEventCreateWithFlags
#define cudaEventDisableTiming                 musaEventDisableTiming
#define cudaEventRecord                        musaEventRecord
#define cudaEventDestroy                       musaEventDestroy

#define cudaStream_t                           musaStream_t
#define cudaStreamCreateWithFlags              musaStreamCreateWithFlags
#define cudaStreamNonBlocking                  musaStreamNonBlocking
#define cudaStreamDestroy                      musaStreamDestroy
#define cudaStreamSynchronize                  musaStreamSynchronize
#define cudaStreamWaitEvent                    musaStreamWaitEvent

#define cudaHostFn_t                           musaHostFn_t
#define cudaLaunchHostFunc                     musaLaunchHostFunc

#define cudaMalloc                             musaMalloc
#define cudaMallocAsync                        musaMallocAsync
#define cudaFree                               musaFree
#define cudaFreeAsync                          musaFreeAsync

#define cudaMemcpyAsync                        musaMemcpyAsync
#define cudaMemcpy2DAsync                      musaMemcpy2DAsync
#define cudaMemcpyHostToDevice                 musaMemcpyHostToDevice
#define cudaMemcpyDeviceToHost                 musaMemcpyDeviceToHost
#define cudaMemsetAsync                        musaMemsetAsync

#define cudaFuncAttributes                     musaFuncAttributes
#define cudaFuncGetAttributes                  musaFuncGetAttributes
#define cudaFuncSetAttribute                   musaFuncSetAttribute
#define cudaFuncAttributeMaxDynamicSharedMemorySize \
                                                musaFuncAttributeMaxDynamicSharedMemorySize
#define cudaLaunchCooperativeKernel            musaLaunchCooperativeKernel

#endif
