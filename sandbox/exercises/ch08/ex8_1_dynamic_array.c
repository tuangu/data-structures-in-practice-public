/*
 * Exercise 8.1: Dynamic Array Growth Strategies
 * Chapter 8: Dynamic Arrays and Amortized Analysis
 *
 * Push 1 million ints into a dynamic array. Compare three growth factors:
 *   A) ×1.5  (Python-style)
 *   B) ×2.0  (classic doubling)
 *   C) ×1.618 (golden ratio — claimed to minimise wasted capacity)
 *
 * Count reallocations for each strategy and measure total counter cost.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "../../common/benchmark.h"

#define PUSH_COUNT 1000000

typedef struct {
    int    *data;
    size_t  size;
    size_t  cap;
    int     reallocs;
    double  growth;
} darray_t;

static void da_init(darray_t *da, double growth) {
    da->data     = malloc(sizeof(int) * 16);
    da->size     = 0;
    da->cap      = 16;
    da->reallocs = 0;
    da->growth   = growth;
    if (!da->data) { perror("malloc"); exit(1); }
}

/* TODO: push val; grow by da->growth factor when full */
static void da_push(darray_t *da, int val) {
    (void)val;
    /* TODO:
     *   if (da->size == da->cap) {
     *       da->cap = (size_t)((double)da->cap * da->growth) + 1;
     *       da->data = realloc(da->data, da->cap * sizeof(int));
     *       da->reallocs++;
     *   }
     *   da->data[da->size++] = val;
     */
}

static void da_free(darray_t *da) { free(da->data); }

static int test_reallocs;

static void workload(double growth) {
    darray_t da;
    da_init(&da, growth);
    for (int i = 0; i < PUSH_COUNT; i++) da_push(&da, i);
    test_reallocs = da.reallocs;
    da_free(&da);
}

int main(void) {
    double factors[] = {1.5, 2.0, 1.618};
    const char *names[] = {"growth_x1.5", "growth_x2.0", "growth_phi"};

    for (int f = 0; f < 3; f++) {
        workload(factors[f]);  /* dry run for realloc count */
        printf("%-20s  reallocs=%d\n", names[f], test_reallocs);
    }

    BENCH_RUN("push_1M_x1.5",  10, 2, { workload(1.5); });
    BENCH_RUN("push_1M_x2.0",  10, 2, { workload(2.0); });
    BENCH_RUN("push_1M_phi",   10, 2, { workload(1.618); });

    return 0;
}
