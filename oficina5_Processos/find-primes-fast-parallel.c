/* Faster prime number finder, parallel implementation */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <sys/wait.h>
#include <unistd.h>

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
    long n = -1, p = -1;
    if (argc > 2) {
        char *notnumber_ptr;
        n = strtol(argv[1], &notnumber_ptr, 10);
        n = *notnumber_ptr == '\0' ? n : -1;
        p = strtol(argv[2], &notnumber_ptr, 10);
        p = *notnumber_ptr == '\0' ? p : -1;
    }

    if (n < 0 || p <= 0) {
        fprintf(stderr, "usage: find-primes-fast-parallel <N> <P>\n"
                        "finds N (large) prime Numbers using P Processes\n");
        return EXIT_FAILURE;
    }

    if (n % p != 0) {
        fprintf(stderr, "n must be multiple of p\n");
        return EXIT_FAILURE;
    }

    // sanity checks
    assert(First_primes_N < First_primes_len);
    assert(First_primes[First_primes_N] > First_large);
    assert(First_primes[First_primes_N-1] < First_large);

    // Initializes the random generator for all subprocesses
    // pseudo_random_seed((uint64_t) clock()); // for varying results each time
    pseudo_random_seed(1729); // for benchmarking with predictable results

    // Creates suprocesses
    pid_t child;
    for (long i=0; i<p-1; i++) {
        // Duplicates the process
        child = fork();
        if (child < 0) {
            fprintf(stderr, "error creating subprocesses\n");
            return EXIT_FAILURE;
        }
        else if (child > 0) {
            // we are in the parent branch: breaks and start processing
            break;
        }
        // We are in the child branch, advances the random generator so the
        // numbers do not overlap...
        pseudo_random_jump();
        // ...and duplicates the process again if necessary
    }

    // Perform computation
    const long n_per_s = n/p;
    long found = 0;
    while (found < n_per_s) {
        uint64_t candidate = pseudo_random_next();
        if (is_prime(candidate)) {
            found++;
            printf("%" PRIu64 "\n", candidate);
            fflush(stdout);
        }
    }

    // Waits for the child to terminate
    if (child > 0) {
        waitpid(child, NULL, 0);
    }
    return EXIT_SUCCESS;
}
