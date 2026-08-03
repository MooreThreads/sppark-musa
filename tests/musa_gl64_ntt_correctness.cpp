// Copyright Supranational LLC
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <vector>

namespace frozen {
constexpr uint64_t modulus = 0xffffffff00000001ULL;
constexpr uint64_t primitive_generator = 7;
constexpr uint32_t lg_domain_size = 20;
constexpr size_t domain_size = size_t{1} << lg_domain_size;
constexpr size_t device_id = 0;  // sole container-local MUSA ordinal
constexpr int order_nn = 0;
constexpr int direction_forward = 0;
constexpr int type_standard = 0;
constexpr uint64_t input_seed = 0x5a228815fccf4dd2ULL;
}  // namespace frozen

struct RustError {
    int code;
    char* message;
};

using compute_ntt_t = RustError (*)(size_t, uint64_t*, uint32_t, int, int, int);

static uint64_t add_mod(uint64_t a, uint64_t b)
{
    return static_cast<uint64_t>((static_cast<unsigned __int128>(a) + b) %
                                 frozen::modulus);
}

static uint64_t sub_mod(uint64_t a, uint64_t b)
{
    return a >= b ? a - b : frozen::modulus - (b - a);
}

static uint64_t mul_mod(uint64_t a, uint64_t b)
{
    return static_cast<uint64_t>((static_cast<unsigned __int128>(a) * b) %
                                 frozen::modulus);
}

static uint64_t pow_mod(uint64_t base, uint64_t exponent)
{
    uint64_t result = 1;
    while (exponent != 0) {
        if (exponent & 1)
            result = mul_mod(result, base);
        base = mul_mod(base, base);
        exponent >>= 1;
    }
    return result;
}

static uint64_t splitmix64(uint64_t value)
{
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

static std::vector<uint64_t> frozen_input()
{
    std::vector<uint64_t> values(frozen::domain_size);
    for (size_t i = 0; i < values.size(); ++i) {
        uint64_t value = splitmix64(frozen::input_seed + i);
        values[i] = value >= frozen::modulus ? value - frozen::modulus : value;
    }
    values[0] = 0;
    values[1] = 1;
    values[2] = frozen::modulus - 1;
    values[3] = frozen::modulus - 2;
    return values;
}

// Independent iterative Cooley-Tukey transform. This code deliberately uses
// no SPPARK headers, roots, field type, or arithmetic implementation.
static void cpu_forward_ntt(std::vector<uint64_t>& values)
{
    const size_t n = values.size();
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            const uint64_t tmp = values[i];
            values[i] = values[j];
            values[j] = tmp;
        }
    }

    const uint64_t root = pow_mod(
        frozen::primitive_generator,
        (frozen::modulus - 1) / static_cast<uint64_t>(n));
    for (size_t len = 2; len <= n; len <<= 1) {
        const uint64_t step = pow_mod(root, n / len);
        for (size_t base = 0; base < n; base += len) {
            uint64_t twiddle = 1;
            for (size_t j = 0; j < len / 2; ++j) {
                const uint64_t even = values[base + j];
                const uint64_t odd = mul_mod(values[base + j + len / 2],
                                             twiddle);
                values[base + j] = add_mod(even, odd);
                values[base + j + len / 2] = sub_mod(even, odd);
                twiddle = mul_mod(twiddle, step);
            }
        }
    }
}

static uint64_t digest(const std::vector<uint64_t>& values)
{
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (uint64_t value : values) {
        for (unsigned int byte = 0; byte < 8; ++byte) {
            hash ^= (value >> (8 * byte)) & 0xff;
            hash *= 0x100000001b3ULL;
        }
    }
    return hash;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s LIBNTT_GL64_MUSA_SO\n", argv[0]);
        return 2;
    }

    std::vector<uint64_t> input = frozen_input();
    std::vector<uint64_t> expected = input;
    cpu_forward_ntt(expected);
    const uint64_t input_digest = digest(input);
    const uint64_t oracle_digest = digest(expected);

    void* library = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (library == nullptr) {
        std::fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 3;
    }
    dlerror();
    auto compute_ntt = reinterpret_cast<compute_ntt_t>(dlsym(library,
                                                              "compute_ntt"));
    if (const char* error = dlerror()) {
        std::fprintf(stderr, "dlsym failed: %s\n", error);
        return 4;
    }

    std::vector<uint64_t> actual = input;
    const RustError status = compute_ntt(
        frozen::device_id, actual.data(), frozen::lg_domain_size,
        frozen::order_nn, frozen::direction_forward, frozen::type_standard);
    if (status.code != 0) {
        std::fprintf(stderr, "compute_ntt failed: code=%d message=%s\n",
                     status.code, status.message == nullptr ? "<none>"
                                                            : status.message);
        std::free(status.message);
        return 5;
    }
    std::free(status.message);

    size_t mismatches = 0;
    size_t noncanonical = 0;
    for (size_t i = 0; i < actual.size(); ++i) {
        noncanonical += actual[i] >= frozen::modulus;
        if (actual[i] != expected[i]) {
            if (mismatches < 8) {
                std::fprintf(stderr,
                             "mismatch[%zu]: gpu=%016llx oracle=%016llx\n",
                             i, static_cast<unsigned long long>(actual[i]),
                             static_cast<unsigned long long>(expected[i]));
            }
            ++mismatches;
        }
    }

    std::printf("backend=MUSA device_local=%zu lg_n=%u n=%zu order=NN "
                "direction=forward type=standard\n",
                frozen::device_id, frozen::lg_domain_size,
                frozen::domain_size);
    std::printf("modulus=%016llx generator=%llu seed=%016llx "
                "input_fn=splitmix64(seed+i)_mod_p_with_boundary_prefix\n",
                static_cast<unsigned long long>(frozen::modulus),
                static_cast<unsigned long long>(frozen::primitive_generator),
                static_cast<unsigned long long>(frozen::input_seed));
    std::printf("input_fnv1a64=%016llx oracle_fnv1a64=%016llx "
                "gpu_fnv1a64=%016llx\n",
                static_cast<unsigned long long>(input_digest),
                static_cast<unsigned long long>(oracle_digest),
                static_cast<unsigned long long>(digest(actual)));
    std::printf("exit_status=%d exact_mismatches=%zu noncanonical=%zu\n",
                status.code, mismatches, noncanonical);

    dlclose(library);
    return mismatches == 0 && noncanonical == 0 ? 0 : 1;
}
