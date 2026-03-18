/*
 * SOLUTION: Exercise 12.1 — Binary Heap vs 4-ary Heap
 * Chapter 12: Priority Queues and Heaps
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A d-ary heap is a generalisation of the binary heap (d=2). Each node has d
 * children instead of 2. This trades:
 *
 *   Fewer levels: height = log_d(n).  d=4 → 10 levels for 100 000 elements
 *                                     d=2 → 17 levels for 100 000 elements
 *     Fewer levels = fewer cache misses on extract-min (which traverses down).
 *
 *   More work per level on extract-min: must find the minimum of d children
 *     instead of 2. For d=4 this is 3 comparisons vs 1 for d=2. The extra
 *     comparisons cost less than a cache miss, so d=4 usually wins.
 *
 *   Insert is unaffected in practice: both sift up from the same leaf,
 *     fewer levels = fewer steps.
 *
 * Index arithmetic for d-ary heap (0-indexed array):
 *   parent(i)      = (i - 1) / d
 *   first_child(i) = d * i + 1
 *   child k of i   = d * i + 1 + k    (k in [0, d-1])
 *
 * Expected result: 4-ary heap is ~10–30% faster for extract-min-heavy
 * workloads on real hardware. Under QEMU the 4-ary heap may be slightly
 * slower due to higher instruction count per level, since there is no
 * cache miss penalty.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define OPS    100000
#define D_BIN  2
#define D_QUAT 4

typedef struct {
    int   *data;
    int    size;
    int    cap;
    int    d;    /* branching factor */
} heap_t;

static void heap_init(heap_t *h, int d, int cap) {
    h->data = malloc((size_t)cap * sizeof(int));
    if (!h->data) { perror("malloc"); exit(1); }
    h->size = 0; h->cap = cap; h->d = d;
}

static void heap_free(heap_t *h) { free(h->data); }

/*
 * STEP 1 — Insert (sift up).
 *
 * Add the new element at the end of the array, then "bubble it up" toward
 * the root by repeatedly swapping it with its parent while it is smaller.
 *
 * STEP 1a: parent index for a d-ary heap.
 *   parent(i) = (i - 1) / d
 *   Integer division makes this work for all d.
 *
 * STEP 1b: sift up loop.
 *   Swap the new element with its parent while:
 *     - the new element is not the root (i > 0), AND
 *     - the new element is smaller than its parent (heap property violated).
 *   Each swap moves the element one level closer to the root.
 *   Worst case: log_d(n) swaps.
 */
static void heap_insert(heap_t *h, int val) {
    /* STEP 1c: place new element at the end */
    int i = h->size++;
    h->data[i] = val;

    /* STEP 1b: sift up until heap property is restored */
    while (i > 0) {
        int parent = (i - 1) / h->d;          /* STEP 1a */
        if (h->data[parent] <= h->data[i])
            break;                             /* heap property holds */
        /* swap with parent */
        int tmp        = h->data[parent];
        h->data[parent] = h->data[i];
        h->data[i]      = tmp;
        i = parent;
    }
}

/*
 * STEP 2 — Extract minimum (sift down).
 *
 * The minimum is always at index 0 (the root of a min-heap).
 * Remove it, place the last element at the root, decrement size, and
 * "sift down" to restore the heap property.
 *
 * STEP 2a: sift down for a d-ary heap.
 *   At each level, find the smallest of the d children. If that child is
 *   smaller than the current element, swap them and descend. Repeat until
 *   no child is smaller or we reach a leaf.
 *
 * STEP 2b: first child index = d * i + 1.
 *   Children of node i are at indices d*i+1, d*i+2, …, d*i+d.
 *
 * STEP 2c: finding the minimum child.
 *   Scan all d children (or fewer at the bottom of the heap) to find the
 *   one with the smallest value.
 */
static int heap_extract_min(heap_t *h) {
    if (h->size == 0) return 0;

    int min = h->data[0];                   /* STEP 2d: save the minimum */
    h->data[0] = h->data[--h->size];        /* move last element to root */

    /* STEP 2a: sift down */
    int i = 0;
    while (1) {
        int first = h->d * i + 1;           /* STEP 2b: first child index */
        if (first >= h->size) break;        /* no children — done */

        /* STEP 2c: find the minimum child among up to d children */
        int min_child = first;
        for (int k = 1; k < h->d && first + k < h->size; k++) {
            if (h->data[first + k] < h->data[min_child])
                min_child = first + k;
        }

        if (h->data[min_child] >= h->data[i])
            break;  /* heap property holds — stop */

        /* swap with the minimum child and descend */
        int tmp           = h->data[i];
        h->data[i]         = h->data[min_child];
        h->data[min_child] = tmp;
        i = min_child;
    }

    return min;
}

static void workload(int d) {
    heap_t h;
    heap_init(&h, d, OPS + 1);
    for (int i = OPS - 1; i >= 0; i--) heap_insert(&h, i);
    volatile int sink = 0;
    for (int i = 0; i < OPS; i++) sink ^= heap_extract_min(&h);
    (void)sink;
    heap_free(&h);
}

int main(void) {
    BENCH_RUN("binary_heap_100k", 20, 5, { workload(D_BIN); });
    BENCH_RUN("4ary_heap_100k",   20, 5, { workload(D_QUAT); });
    return 0;
}
