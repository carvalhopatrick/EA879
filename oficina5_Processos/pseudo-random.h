/* A pseudo-random generator based on David Blackman and Sebastiano Vigna's
   xorshiro; their original code, for reference is in xorshiro.c and
   splitmix64.c the latter being used to seed the former. This basically just
   adds header guards and friendlier/safer names to their code. */

#ifndef PSEUDO_RANDOM_H
#define PSEUDO_RANDOM_H

#include <stdint.h>

/* This is xoshiro256** 1.0, a rock-solid pseudo-random generators. It has
   excellent (sub-ns) speed, a state (256 bits) that is large enough for any
   parallel application, and it passes all known tests of pseudo-randomness */

uint64_t pseudo_random_next(void);

/* Jump function for the generator. It is equivalent to 2^128 calls to
   pseudo_random_next(); it can be used to generate 2^128 non-overlapping
   subsequences for parallel computations. */

void pseudo_random_jump(void);

/* Long-jump function for the generator. It is equivalent to 2^192
   calls to pseudo_random_next(); it can be used to generate 2^64 starting
   points, from each of which pseudo_random_jump() will generate 2^64
   non-overlapping subsequences for parallel distributed computations. */
void pseudo_random_long_jump(void);

/* Recommended initialization of the 256-bit state from a 64-bit seed */
void pseudo_random_seed(uint64_t seed);

#endif
