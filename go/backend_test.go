// Copyright Supranational LLC
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

package sppark

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func fakeCompiler(t *testing.T, name string) string {
	t.Helper()
	path := filepath.Join(t.TempDir(), name)
	if err := os.WriteFile(path, []byte("#!/bin/sh\nexit 0\n"), 0755); err != nil {
		t.Fatal(err)
	}
	return path
}

func TestExplicitMusaSelection(t *testing.T) {
	t.Setenv("NVCC", fakeCompiler(t, "nvcc"))
	t.Setenv("HIPCC", fakeCompiler(t, "hipcc"))
	mcc := fakeCompiler(t, "mcc")
	t.Setenv("MCC", mcc)
	t.Setenv("SPPARK_BACKEND", "musa")

	backend, err := selectGpuBackend()
	if err != nil {
		t.Fatal(err)
	}
	if backend.name != "musa" || backend.compiler != mcc {
		t.Fatalf("selected %#v, want MUSA compiler %q", backend, mcc)
	}
}

func TestExplicitCudaAndRocmSelection(t *testing.T) {
	compilers := map[string]struct{ env, path string }{
		"cuda": {"NVCC", fakeCompiler(t, "nvcc")},
		"rocm": {"HIPCC", fakeCompiler(t, "hipcc")},
	}
	for name, compiler := range compilers {
		t.Run(name, func(t *testing.T) {
			t.Setenv(compiler.env, compiler.path)
			t.Setenv("SPPARK_BACKEND", name)
			backend, err := selectGpuBackend()
			if err != nil {
				t.Fatal(err)
			}
			if backend.name != name || backend.compiler != compiler.path {
				t.Fatalf("selected %#v, want %s compiler %q", backend, name, compiler.path)
			}
		})
	}
}

func TestAutomaticSelectionRejectsMultipleBackends(t *testing.T) {
	t.Setenv("NVCC", fakeCompiler(t, "nvcc"))
	t.Setenv("HIPCC", fakeCompiler(t, "hipcc"))
	t.Setenv("MCC", fakeCompiler(t, "mcc"))
	t.Setenv("SPPARK_BACKEND", "")

	_, err := selectGpuBackend()
	if err == nil || !strings.Contains(err.Error(), "multiple GPU compilers") {
		t.Fatalf("got %v, want multiple compiler failure", err)
	}
}

func TestSelectionRejectsUnknownBackend(t *testing.T) {
	t.Setenv("SPPARK_BACKEND", "other")
	_, err := selectGpuBackend()
	if err == nil || !strings.Contains(err.Error(), "unknown SPPARK_BACKEND") {
		t.Fatalf("got %v, want unknown backend failure", err)
	}
}
