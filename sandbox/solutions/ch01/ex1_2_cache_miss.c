/*
 * SOLUTION: Exercise 1.2 — Sequential vs Random Array Access
 * Chapter 1: The Performance Gap
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * Both functions sum the same 1 M integer array. The only difference is the
 * access order:
 *
 *   sequential_sum: arr[0], arr[1], arr[2], …
 *     → each load is adjacent to the previous. The hardware prefetcher
 *       detects the stride-1 pattern and pre-fetches the next cache lines
 *       before they are needed. Almost no stall time.
 *
 *   random_sum: arr[rand_idx[0]], arr[rand_idx[1]], …
 *     → the address of the next element is unknown until the current load
 *       completes. The prefetcher cannot help. Every access to a "cold"
 *       cache line costs a full DRAM access (~100–200 ns on modern hardware).
 *
 * Under QEMU rdcycle counts instructions, not wall-clock time, so the
 * instruction counts are nearly identical (both execute the same number
 * of load/add instructions). On real hardware the random version is
 * typically 5–20× slower.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../common/benchmark.h"

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

/*
 * STEP 1 — Sequential sum.
 *
 * Iterate i from 0 to N-1 and accumulate arr[i].
 *
 * The access pattern is perfectly linear: each element is 4 bytes after the
 * last. A typical 64-byte cache line holds 16 int32_t values, so we pay one
 * cache-miss cost per 16 elements. The hardware prefetcher loads the next
 * line while we are still processing the current one.
 *
 * Use int64_t for the accumulator to avoid overflow (N * (N-1)/2 > INT_MAX).
 */
static int64_t sequential_sum(void) {
    int64_t sum = 0;

    /* STEP 1a: straightforward linear scan — most cache-friendly possible */
    for (int i = 0; i < N; i++)
        sum += arr[i];

    return sum;
}

/*
 * STEP 2 — Random sum.
 *
 * Iterate i from 0 to N-1 but read arr[rand_idx[i]] instead of arr[i].
 *
 * rand_idx[] is a full permutation of [0, N), so every element of arr[] is
 * read exactly once — the total work is identical to sequential_sum.
 * However, the memory address of each access is unpredictable. With N = 1 M
 * elements, the array spans 4 MB, well above any L2 or L3 cache on most
 * embedded targets. Every access is therefore likely to be a full DRAM miss.
 *
 * Note: we also read rand_idx[i] sequentially (predictable), so the *index*
 * array has good cache behaviour — it is the *data* lookup that is random.
 */
static int64_t random_sum(void) {
    int64_t sum = 0;

    /* STEP 2a: indirect access — rand_idx[i] determines which arr slot to read */
    for (int i = 0; i < N; i++)
        sum += arr[rand_idx[i]];

    return sum;
}

int main(void) {
    setup();

    volatile int64_t sink = 0;

    BENCH_RUN("sequential_access", 20, 5, { sink ^= sequential_sum(); });
    BENCH_RUN("random_access",     20, 5, { sink ^= random_sum(); });

    (void)sink;
    return 0;
}
