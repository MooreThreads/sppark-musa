// Host-only bit-vector and layout checks for the portable MUSA Goldilocks path.
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#define SPPARK_GL64_HOST_TEST 1
#include <ff/gl64_t.musa>

static uint64_t ref_add(uint64_t a, uint64_t b)
{
    return static_cast<uint64_t>((static_cast<unsigned __int128>(a) + b) %
                                 gl64_t::MOD);
}

static uint64_t ref_sub(uint64_t a, uint64_t b)
{
    return static_cast<uint64_t>((static_cast<unsigned __int128>(a) +
                                  gl64_t::MOD - b) % gl64_t::MOD);
}

static uint64_t ref_mul(uint64_t a, uint64_t b)
{
    return static_cast<uint64_t>((static_cast<unsigned __int128>(a) * b) %
                                 gl64_t::MOD);
}

int main()
{
    static_assert(sizeof(gl64_t) == sizeof(uint64_t));
    static_assert(alignof(gl64_t) == alignof(uint64_t));
    static_assert(std::is_trivially_copyable<gl64_t>::value);
    static_assert(gl64_t::degree == 1 && gl64_t::nbits == 64);
    static_assert(gl64_t::bit_length() == 64);

    const uint64_t vectors[] = {
        0, 1, 2, 0xffffffffULL, 0x100000000ULL,
        0x7fffffffffffffffULL, gl64_t::MOD / 2,
        gl64_t::MOD - 2, gl64_t::MOD - 1,
    };

    for (uint64_t a : vectors) {
        gl64_t layout(a);
        assert(layout.len() == 1);
        assert(layout[0] == a);
        uint64_t stored = 0;
        layout.store(&stored);
        assert(stored == a);

        for (uint64_t b : vectors) {
            assert(static_cast<uint64_t>(gl64_t(a) + gl64_t(b)) == ref_add(a, b));
            assert(static_cast<uint64_t>(gl64_t(a) - gl64_t(b)) == ref_sub(a, b));
            assert(static_cast<uint64_t>(gl64_t(a) * gl64_t(b)) == ref_mul(a, b));
            assert(gl64_t::csel(gl64_t(a), gl64_t(b), 1) == gl64_t(a));
            assert(gl64_t::csel(gl64_t(a), gl64_t(b), 0) == gl64_t(b));
        }

        assert(static_cast<uint64_t>(-gl64_t(a)) == (a ? gl64_t::MOD - a : 0));
        assert(static_cast<uint64_t>(gl64_t(a) << unsigned(1)) == ref_add(a, a));
        assert(static_cast<uint64_t>(gl64_t(a) >> unsigned(1)) ==
               static_cast<uint64_t>((static_cast<unsigned __int128>(a) +
                                      (a & 1 ? gl64_t::MOD : 0)) >> 1));
    }

    uint64_t state = 0x243f6a8885a308d3ULL;
    for (unsigned i = 0; i < 4096; i++) {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        uint64_t a = state % gl64_t::MOD;
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        uint64_t b = state % gl64_t::MOD;
        assert(static_cast<uint64_t>(gl64_t(a) + gl64_t(b)) == ref_add(a, b));
        assert(static_cast<uint64_t>(gl64_t(a) - gl64_t(b)) == ref_sub(a, b));
        assert(static_cast<uint64_t>(gl64_t(a) * gl64_t(b)) == ref_mul(a, b));
    }

    const uint64_t inverse_vectors[] = {1, 2, 7, gl64_t::MOD - 1};
    for (uint64_t a : inverse_vectors) {
        assert(gl64_t(a) * gl64_t(a).reciprocal() == gl64_t::one());
        gl64_t root = gl64_t(a).heptaroot();
        assert((root ^ 7) == gl64_t(a));
    }
}
