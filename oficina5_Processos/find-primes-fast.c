/* Faster prime number finder */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "pseudo-random.h"

// Test primes up to 9239, the next would be 9241
const size_t First_primes_N = 1145;
const uint64_t First_large = 9240; // primorial(5)==2310, 9240==4*2310

#include "first-primes.inc"

bool is_prime(const uint64_t candidate) {
    if (candidate <= 1) {
        return false;
    }
    // First, check if number is divisible by small primes
    for (size_t i=0; i<First_primes_N; i++) {
        if (candidate % First_primes[i] == 0) return false;
    }

    // Then check large divisors
    const uint64_t max_divisor = (uint64_t) sqrtl(candidate);
    for (uint64_t divisor=First_large; divisor<=max_divisor; divisor+=2310) {
        if (
            #include "prime-test-skip2-3-5-7-11.inc"
           ) return false;
    }
    return true;
}


int main(int argc, char *argv[]) {
    long n = -1;
    if (argc > 1) {
        char *notnumber_ptr;
        n = strtol(argv[1], &notnumber_ptr, 10);
        n = *notnumber_ptr == '\0' ? n : -1;
    }

    if (n < 0) {
        fprintf(stderr, "usage: find-primes-fast <N>\n"
                        "finds N (large) prime numbers\n");
        return EXIT_FAILURE;
    }

    // sanity checks
    assert(First_primes_N < First_primes_len);
    assert(First_primes[First_primes_N] > First_large);
    assert(First_primes[First_primes_N-1] < First_large);

    // pseudo_random_seed((uint64_t) clock()); // for varying results each time
    pseudo_random_seed(1729); // for benchmarking with predictable results

    long found = 0;
    while (found < n) {
        uint64_t candidate = pseudo_random_next();
        if (is_prime(candidate)) {
            found++;
            printf("%" PRIu64 "\n", candidate);
            fflush(stdout);
        }
    }

    return EXIT_SUCCESS;
}
