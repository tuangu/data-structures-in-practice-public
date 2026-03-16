/*
 * Exercise 1.2: Sequential vs Random Array Access
 * Chapter 1: The Performance Gap
 *
 * Compare sequential traversal (cache-friendly) with random traversal
 * (cache-hostile) over the same array. On real hardware the difference
 * is dramatic; under QEMU instruction counts will be nearly equal
 * (no cache simulation), but the exercise still builds and runs correctly.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define N (1 << 20)   /* 1 M elements — well above typical L2 */

static int32_t arr[N];
static int     rand_idx[N];

static void setup(void) {
    for (int i = 0; i < N; i++) arr[i] = i;

    /* Fisher-Yates shuffle to build a random permutation index */
    for (int i = 0; i < N; i++) rand_idx[i] = i;
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = rand_idx[i];
        rand_idx[i] = rand_idx[j];
        rand_idx[j] = tmp;
    }
}

/* TODO: sum arr[] in sequential order */
static int64_t sequential_sum(void) {
    return 0; /* TODO */
}

/* TODO: sum arr[] in the order given by rand_idx[] */
static int64_t random_sum(void) {
    return 0; /* TODO */
}

int main(void) {
    setup();

    volatile int64_t sink = 0;

    BENCH_RUN("sequential_access", 20, 5, { sink ^= sequential_sum(); });
    BENCH_RUN("random_access",     20, 5, { sink ^= random_sum(); });

    (void)sink;
    return 0;
}
