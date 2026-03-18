/*
 * SOLUTION: Exercise 4.1 — Row-Major vs Column-Major Matrix Traversal
 * Chapter 4: Arrays and Cache Locality
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * C stores 2-D arrays in *row-major* order: matrix[r][0], matrix[r][1], …,
 * matrix[r][COLS-1] are contiguous in memory, then matrix[r+1][0], etc.
 *
 * Row-major traversal (r outer, c inner):
 *   Reads matrix[r][0], [r][1], [r][2], … — sequential addresses.
 *   A 64-byte cache line holds 16 ints (4 bytes each). One miss per 16 reads.
 *   The hardware prefetcher easily predicts the stride-1 pattern.
 *
 * Column-major traversal (c outer, r inner):
 *   Reads matrix[0][c], matrix[1][c], matrix[2][c], … — each element is
 *   COLS * sizeof(int) = 512 * 4 = 2048 bytes apart. For a 512×512 matrix
 *   (1 MB) this is far beyond any L1 cache line, so every access is a miss.
 *   With ROWS=512 each column traversal has 512 cache misses instead of ~32.
 *
 * Expected result on real hardware:
 *   col_major is ~10–50× slower than row_major.
 *   Under QEMU instruction counts are identical (no cache simulation).
 */
#include <stdio.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define ROWS 512
#define COLS 512

static int matrix[ROWS][COLS];

static void setup(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            matrix[r][c] = r * COLS + c;
}

/*
 * STEP 1 — Row-major sum (cache-friendly).
 *
 * The outer loop iterates over rows, the inner loop over columns.
 * Because C stores rows contiguously, successive reads of matrix[r][c],
 * matrix[r][c+1], … access adjacent memory addresses.
 *
 * STEP 1a: use int64_t to avoid overflow (512×512 elements, values up to
 *   512*512-1 ≈ 262 143; sum ≈ 512×512 × 131 071 ≈ 34 × 10^9 > INT_MAX).
 */
static int64_t row_major_sum(void) {
    int64_t sum = 0;

    /* STEP 1b: r outer — each complete inner pass reads one contiguous row */
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            sum += matrix[r][c];

    return sum;
}

/*
 * STEP 2 — Column-major sum (cache-hostile).
 *
 * The outer loop iterates over columns, the inner loop over rows.
 * Successive reads of matrix[0][c], matrix[1][c], … are 2048 bytes apart —
 * each is on a different cache line, and in a cold cache each is a DRAM miss.
 *
 * STEP 2a: the code is a trivial swap of the loop order — same operations,
 *   same result, but completely different memory access pattern.
 */
static int64_t col_major_sum(void) {
    int64_t sum = 0;

    /* STEP 2b: c outer — each inner step jumps COLS * sizeof(int) bytes */
    for (int c = 0; c < COLS; c++)
        for (int r = 0; r < ROWS; r++)
            sum += matrix[r][c];

    return sum;
}

int main(void) {
    setup();

    volatile int64_t sink = 0;
    BENCH_RUN("row_major", 100, 10, { sink ^= row_major_sum(); });
    BENCH_RUN("col_major", 100, 10, { sink ^= col_major_sum(); });
    (void)sink;
    return 0;
}
