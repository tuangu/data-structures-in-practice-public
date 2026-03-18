/*
 * SOLUTION: Exercise 8.1 — Dynamic Array Growth Strategies
 * Chapter 8: Dynamic Arrays and Amortised Analysis
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A dynamic array starts at some capacity and doubles (or grows by some factor)
 * when it runs out of space. The trade-off between growth factor and memory
 * waste:
 *
 *   ×2.0 (doubling):
 *     Fewest reallocations — log2(N) total for N pushes.
 *     But wastes up to 50% of allocated capacity just after a resize.
 *     Amortised O(1) per push is provably tight.
 *
 *   ×1.5 (Python-style):
 *     ~50% more reallocations than ×2.0.
 *     Wastes at most 33% of capacity. Better when memory is constrained.
 *
 *   ×1.618 (golden ratio / phi):
 *     Sometimes claimed to minimise wasted capacity while keeping reallocation
 *     count low. In practice the difference from ×1.5 is negligible; it is
 *     more a curiosity than an engineering recommendation.
 *
 * Key question: does more frequent reallocation slow things down?
 * Each realloc copies all existing elements. With doubling, total bytes copied
 * across all resizes = O(N) (geometric series). Same for any constant factor.
 * The factor only changes the constant, not the asymptotic class.
 *
 * Expected results: ×2.0 has the fewest reallocations. Speed differences
 * between strategies are small — dominated by memory traffic, not realloc count.
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

/*
 * STEP 1 — Push with configurable growth factor.
 *
 * STEP 1a: full check.
 *   When size reaches cap, the array is full. We must resize before writing
 *   the new element.
 *
 * STEP 1b: compute new capacity.
 *   new_cap = (size_t)(current_cap * growth_factor) + 1
 *   The +1 ensures we always grow by at least 1 even when cap is very small
 *   and a fractional growth factor would otherwise leave cap unchanged.
 *
 * STEP 1c: realloc.
 *   realloc() may extend the existing allocation in place (no copy) or
 *   allocate a new block and copy. We cannot assume in-place extension.
 *   We count reallocations so the learner can compare strategies.
 *
 * STEP 1d: store and advance size.
 */
static void da_push(darray_t *da, int val) {
    /* STEP 1a: do we need more space? */
    if (da->size == da->cap) {
        /* STEP 1b: grow by the configured factor */
        da->cap = (size_t)((double)da->cap * da->growth) + 1;

        /* STEP 1c: resize the backing array */
        da->data = realloc(da->data, da->cap * sizeof(int));
        if (!da->data) { perror("realloc"); exit(1); }
        da->reallocs++;
    }

    /* STEP 1d: write the new element and advance the size */
    da->data[da->size++] = val;
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
        workload(factors[f]);
        printf("%-20s  reallocs=%d\n", names[f], test_reallocs);
    }

    BENCH_RUN("push_1M_x1.5",  10, 2, { workload(1.5); });
    BENCH_RUN("push_1M_x2.0",  10, 2, { workload(2.0); });
    BENCH_RUN("push_1M_phi",   10, 2, { workload(1.618); });

    return 0;
}
