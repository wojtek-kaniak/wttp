#include <stdint.h>
#include <sys/random.h>
#include "random.h"
#include "common.h"

typedef struct RngState
{
	uint64_t low;
	uint64_t high;
} RngState;

thread_local RngState state;
thread_local bool initialized = false;

void random_init_thread()
{
	uint64_t entropy[2];
	getrandom(&entropy, sizeof(entropy), 0);

	state.low = entropy[0];
	state.high = entropy[1];

	if (state.low == 0)
		state.low = 42;

	if (state.high == 0)
		state.high = 42;

	initialized = true;
}

void random_init_thread_with_seed(uint64_t seed)
{
	state.low = (uint32_t)seed;
	state.high = seed >> 32;

	if (state.low == 0)
		state.low = 42;

	if (state.high == 0)
		state.high = 42;

	initialized = true;
}

// XorShift128+ RNG
uint64_t random_next()
{
	DEBUG_ASSERT(initialized);

	uint64_t t = state.low;
	uint64_t const s = state.high;
	state.low = s;
	t ^= t << 23;
	t ^= t >> 18;
	t ^= s ^ (s >> 5);
	state.high = t;
	return t + s;
}
