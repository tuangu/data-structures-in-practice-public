/*
 * SOLUTION: Exercise 2.1 — Cache Line Size Detection via Stride Access
 * Chapter 2: Memory Hierarchy
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * The key insight: a cache line is the atomic unit of data that moves between
 * DRAM and the cache. When you touch any byte in a cache line, the entire line
 * is loaded — all other bytes in the line are then "free".
 *
 * Stride experiment:
 *   stride = 1 byte  → touch every byte → N accesses, N/64 cache misses
 *   stride = 2 bytes → touch every other byte → N/2 accesses, N/64 misses
 *   ...
 *   stride = 64 bytes → touch one byte per line → N/64 accesses, N/64 misses
 *   stride = 128 bytes → touch one byte per 2 lines → N/128 accesses, N/128 misses
 *
 * As stride grows from 1 to 64, the *work* (number of accesses) halves each
 * step but the *cache misses* stay constant — so cost per access increases.
 * Past 64 bytes both access count AND miss count drop together — total time
 * falls again. The inflection point where "time stops improving" pinpoints
 * the cache line size.
 *
 * NOTE: Under QEMU there is no cache simulation, so instruction count will
 * drop monotonically with stride (fewer loop iterations). Run on real hardware
 * (x86-64) to observe the plateau at stride = cache line size (~64 bytes).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define ARRAY_BYTES (4 * 1024 * 1024)   /* 4 MiB — well above any L3 */
#define MAX_STRIDE  512                  /* bytes */
#define ITERATIONS  100

static char *buf;

static void setup(void) {
    buf = malloc(ARRAY_BYTES);
    if (!buf) { perror("malloc"); exit(1); }
    for (int i = 0; i < ARRAY_BYTES; i++) buf[i] = (char)i;
}

/*
 * STEP 1 — Stride access function.
 *
 * Walk buf[] touching one byte every `stride` bytes, accumulate into sum.
 * The volatile qualifier on the return type prevents the compiler from
 * eliminating the loop as dead code.
 *
 * Implementation detail: we iterate by byte offset so that `stride` is
 * always measured in bytes regardless of element type. The cast to
 * (unsigned char) avoids sign-extension issues when summing into int64_t.
 *
 * STEP 1a: why accumulate the sum?
 *   Without using the loaded value the compiler or CPU may speculate/hoist
 *   the loads. The sum forces each load to complete before the next iteration
 *   (data dependency chain), making the measurement accurate.
 */
static volatile int64_t stride_access(int stride) {
    int64_t sum = 0;

    /* STEP 1b: step through the buffer in increments of `stride` bytes */
    for (int i = 0; i < ARRAY_BYTES; i += stride)
        sum += (unsigned char)buf[i];

    return sum;
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
