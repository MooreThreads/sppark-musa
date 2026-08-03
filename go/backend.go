// Copyright Supranational LLC
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

package sppark

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
)

type gpuBackend struct {
	name     string
	compiler string
}

func selectGpuBackend() (gpuBackend, error) {
	candidates := []struct{ name, env, command string }{
		{"cuda", "NVCC", "nvcc"},
		{"rocm", "HIPCC", "hipcc"},
		{"musa", "MCC", "mcc"},
	}
	requested := strings.ToLower(os.Getenv("SPPARK_BACKEND"))
	if requested != "" && requested != "cuda" && requested != "rocm" && requested != "musa" {
		return gpuBackend{}, fmt.Errorf("unknown SPPARK_BACKEND %q", requested)
	}

	var found []gpuBackend
	for _, candidate := range candidates {
		command := candidate.command
		if value, ok := os.LookupEnv(candidate.env); ok {
			command = value
		}
		compiler, err := exec.LookPath(command)
		if err == nil && (requested == "" || requested == candidate.name) {
			found = append(found, gpuBackend{candidate.name, compiler})
		}
	}
	if len(found) == 0 {
		if requested == "" {
			return gpuBackend{}, fmt.Errorf("no CUDA, ROCm, or MUSA compiler found")
		}
		return gpuBackend{}, fmt.Errorf("%s compiler selected but not found", requested)
	}
	if len(found) != 1 {
		return gpuBackend{}, fmt.Errorf("multiple GPU compilers found; set SPPARK_BACKEND to cuda, rocm, or musa")
	}
	return found[0], nil
}
