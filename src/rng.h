#ifndef LU_RNG_H
#define LU_RNG_H

#include <cstdint>

#include "internal/constexpr.h"

namespace lu
{
namespace rng
{
// The following xorshifts are from: 
// Marsaglia, G. (2003). Xorshift RNGs. Journal of Statistical Software, 8(14), 1–6. https://doi.org/10.18637/jss.v008.i14
// (secondary from wikipedia)

/* The state must be initialized to non-zero */

struct xorshift32_state
{
    static LU_CONSTEXPR uint32_t DEFAULT_SEED = 2463534242;

    xorshift32_state() : state(DEFAULT_SEED) {}

    uint32_t state;
};


LU_INLINE uint32_t xorshift32(struct xorshift32_state *state)
{
	uint32_t x = state->state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->state = x;
}

struct xorshift64_state
{
    static LU_CONSTEXPR uint64_t DEFAULT_SEED = 88172645463325252LL;

    xorshift64_state() : state(DEFAULT_SEED) {}

    uint64_t state;
};

LU_INLINE uint64_t xorshift64(struct xorshift64_state *state)
{
	uint64_t x = state->state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return state->state = x;
}

}
}

#endif // LU_RNG_H
