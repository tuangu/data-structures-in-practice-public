/*
 * Exercise 2.1: Cache Line Size Detection via Stride Access
 * Chapter 2: Memory Hierarchy
 *
 * Access a large array with varying strides and observe when performance
 * stops improving. The inflection point marks the cache line boundary.
 *
 * NOTE: Under QEMU user-mode there is no real cache simulation; this
 * exercise is most meaningful on bare metal or x86_64. The skeleton
 * compiles and runs correctly under QEMU as a structural check.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define ARRAY_BYTES (4 * 1024 * 1024)   /* 4 MiB — well above any L3 */
#define MAX_STRIDE  512                  /* bytes */
#define ITERATIONS  100

static char *buf;

static void setup(void) {
    buf = malloc(ARRAY_BYTES);
    if (!buf) { perror("malloc"); exit(1); }
    for (int i = 0; i < ARRAY_BYTES; i++) buf[i] = (char)i;
}

/* TODO: read every `stride` bytes of buf, accumulate a sum and return it */
static volatile int64_t stride_access(int stride) {
    (void)stride;
    return 0; /* TODO */
}

int main(void) {
    setup();

    printf("%-12s  %s\n", "stride(B)", "mean_counter");
    for (int stride = 1; stride <= MAX_STRIDE; stride *= 2) {
        uint64_t samples[ITERATIONS];
        for (int i = 0; i < ITERATIONS; i++) {
            uint64_t t0 = read_counter();
            (void)stride_access(stride);
            uint64_t t1 = read_counter();
            samples[i] = t1 - t0;
        }
        bench_stats_t st;
        bench_stats(samples, ITERATIONS, &st);
        printf("%-12d  %.0f\n", stride, st.mean);
    }

    free(buf);
    return 0;
}
