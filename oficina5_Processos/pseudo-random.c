/* A pseudo-random generator based on David Blackman and Sebastiano Vigna's
   xorshiro; their original code, for reference is in xorshiro.c and
   splitmix64.c the latter being used to seed the former. This basically just
   adds header guards and friendlier/safer names to their code. */

#include <stdint.h>

#include "pseudo-random.h"

/* This is xoshiro256** 1.0, one of our all-purpose, rock-solid
   generators. It has excellent (sub-ns) speed, a state (256 bits) that is
   large enough for any parallel application, and it passes all tests we
   are aware of.

   For generating just floating-point numbers, xoshiro256+ is even faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill pseudo_random_state. */

uint64_t pseudo_random_state[4];

static inline uint64_t pseudo_random_rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t pseudo_random_next(void) {
    const uint64_t result = pseudo_random_rotl(pseudo_random_state[1]*5, 7) * 9;

    const uint64_t t = pseudo_random_state[1] << 17;

    pseudo_random_state[2] ^= pseudo_random_state[0];
    pseudo_random_state[3] ^= pseudo_random_state[1];
    pseudo_random_state[1] ^= pseudo_random_state[2];
    pseudo_random_state[0] ^= pseudo_random_state[3];

    pseudo_random_state[2] ^= t;

    pseudo_random_state[3] = pseudo_random_rotl(pseudo_random_state[3], 45);

    return result;
}

/* This is the jump function for the generator. It is equivalent
   to 2^128 calls to pseudo_random_next(); it can be used to generate 2^128
   non-overlapping subsequences for parallel computations. */

void pseudo_random_jump(void) {
    static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c,
        0xa9582618e03fc9aa, 0x39abdc4529b1661c };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
        for(int b = 0; b < 64; b++) {
            if (JUMP[i] & UINT64_C(1) << b) {
                s0 ^= pseudo_random_state[0];
                s1 ^= pseudo_random_state[1];
                s2 ^= pseudo_random_state[2];
                s3 ^= pseudo_random_state[3];
            }
            pseudo_random_next();
        }

    pseudo_random_state[0] = s0;
    pseudo_random_state[1] = s1;
    pseudo_random_state[2] = s2;
    pseudo_random_state[3] = s3;
}

/* This is the long-jump function for the generator. It is equivalent to 2^192
   calls to pseudo_random_next(); it can be used to generate 2^64 starting
   points, from each of which jump() will generate 2^64 non-overlapping
   subsequences for parallel distributed computations. */

void pseudo_random_long_jump(void) {
    static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf,
        0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
        for(int b = 0; b < 64; b++) {
            if (LONG_JUMP[i] & UINT64_C(1) << b) {
                s0 ^= pseudo_random_state[0];
                s1 ^= pseudo_random_state[1];
                s2 ^= pseudo_random_state[2];
                s3 ^= pseudo_random_state[3];
            }
            pseudo_random_next();
        }

    pseudo_random_state[0] = s0;
    pseudo_random_state[1] = s1;
    pseudo_random_state[2] = s2;
    pseudo_random_state[3] = s3;
}

/* Code slightly adapted from Blackman and Vigna's, using a local static state.
   First call should have the seed value, and subsequent calls should set seed
   to zero. Since local static variables are initialized to 0, the code always
   works predictably, even when the first seed is 0 */
uint64_t _pseudo_random_splitmix64(uint64_t seed) {
    static uint64_t x;
    if (seed>0) x=seed;
    uint64_t z = (x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

/* This code, not from Blackman and Vigna's, implements the state
   initialization they recommend */
void pseudo_random_seed(uint64_t seed) {
    pseudo_random_state[0] = _pseudo_random_splitmix64(seed);
    pseudo_random_state[1] = _pseudo_random_splitmix64(0);
    pseudo_random_state[2] = _pseudo_random_splitmix64(0);
    pseudo_random_state[3] = _pseudo_random_splitmix64(0);
}
