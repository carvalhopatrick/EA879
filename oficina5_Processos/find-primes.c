/* Slow prime number finder */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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
    long n = -1;
    if (argc > 1) {
        char *notnumber_ptr;
        n = strtol(argv[1], &notnumber_ptr, 10);
        n = *notnumber_ptr == '\0' ? n : -1;
    }

    if (n < 0) {
        fprintf(stderr, "usage: find-primes <N>\n"
                        "finds N (large) prime Numbers\n");
        return EXIT_FAILURE;
    }

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
