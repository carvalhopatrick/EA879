/* Slow prime number finder, parallel implementation */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <sys/wait.h>
#include <unistd.h>

#include "pseudo-random.h"


bool is_prime(uint64_t candidate) {
    if (candidate <= 1) {
        return false;
    }
    const uint64_t max_divisor_to_test = (uint64_t) sqrtl(candidate);
    for (uint64_t divisor=2; divisor<=max_divisor_to_test; divisor++) {
        if (candidate % divisor == 0) {
            return false;
        }
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
        fprintf(stderr, "usage: find-primes-parallel <N> <P>\n"
                        "finds N (large) prime Numbers using P Processes\n");
        return EXIT_FAILURE;
    }

    if (n % p != 0) {
        fprintf(stderr, "n must be multiple of p\n");
        return EXIT_FAILURE;
    }

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
