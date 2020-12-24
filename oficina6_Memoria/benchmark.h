/* Ancillary functions to measure runtime. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define  BENCHMARK_STACK_SIZE  (64)
static size_t  benchmark_stack_n = 0;
static clock_t benchmark_stack[BENCHMARK_STACK_SIZE];

static void benchmark_push(void) {
    if (benchmark_stack_n >= BENCHMARK_STACK_SIZE) {
        fprintf(stderr, "benchmark_push() stack overflow\n");
        exit(1);
    }
    benchmark_stack[benchmark_stack_n] = clock();
    benchmark_stack_n++;
}

static double benchmark_pop(void) {
    clock_t benchmark_stop = clock();
    if (benchmark_stack_n <= 0) {
        fprintf(stderr, "benchmark_pop() stack underflow\n");
        exit(1);
    }
    benchmark_stack_n--;
    clock_t benchmark_start = benchmark_stack[benchmark_stack_n];
    return ((double)(benchmark_stop-benchmark_start))/CLOCKS_PER_SEC*1e9;
}

static double benchmark_resolution(void) {
    return 1.0/CLOCKS_PER_SEC*1e9;
}
