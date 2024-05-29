// Thread local fast RNG (not cryptographically secure)

#if !defined (INCL_RANDOM_H)

#include <stdint.h>

/// Initialize the RNG on the current thread using system entropy
void random_init_thread();

/// Initialize the RNG on the current thread using a specific seed
void random_init_thread_with_seed(uint64_t seed);

/// Generate a pseudorandom `uint64_t` value using the current thread's RNG
uint64_t random_next();

#endif
