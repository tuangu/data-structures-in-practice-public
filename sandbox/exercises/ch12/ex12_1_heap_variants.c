/*
 * Exercise 12.1: Binary Heap vs 4-ary Heap
 * Chapter 12: Priority Queues and Heaps
 *
 * Measure 100 000 insert + extract-min operations for:
 *   A) Binary heap (d=2)
 *   B) 4-ary heap  (d=4, fewer levels, better cache utilization)
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
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

/* TODO: implement heap_insert(h, val) for d-ary heap */
static void heap_insert(heap_t *h, int val) {
    (void)h; (void)val; /* TODO */
}

/* TODO: implement heap_extract_min(h) for d-ary heap */
static int heap_extract_min(heap_t *h) {
    (void)h;
    return 0; /* TODO */
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
