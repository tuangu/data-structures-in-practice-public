/*
 * Exercise 4.1: Row-Major vs Column-Major Matrix Traversal
 * Chapter 4: Arrays and Cache Locality
 *
 * A 512×512 int matrix is traversed two ways:
 *   - Row-major:    matrix[r][c] — sequential in memory, cache-friendly
 *   - Column-major: matrix[r][c] iterated c-outer — stride = 512 ints
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
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

/* TODO: sum matrix elements in row-major order (r outer, c inner) */
static int64_t row_major_sum(void) {
    return 0; /* TODO */
}

/* TODO: sum matrix elements in column-major order (c outer, r inner) */
static int64_t col_major_sum(void) {
    return 0; /* TODO */
}

int main(void) {
    setup();

    volatile int64_t sink = 0;
    BENCH_RUN("row_major", 100, 10, { sink ^= row_major_sum(); });
    BENCH_RUN("col_major", 100, 10, { sink ^= col_major_sum(); });
    (void)sink;
    return 0;
}
