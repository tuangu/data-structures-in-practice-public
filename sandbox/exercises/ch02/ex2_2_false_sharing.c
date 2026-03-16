/*
 * Exercise 2.2: False Sharing Between Threads
 * Chapter 2: Memory Hierarchy
 *
 * Two threads each increment their own counter 10 million times.
 * Case A: counters are adjacent (share a cache line) — false sharing.
 * Case B: counters are padded to separate cache lines.
 *
 * On real hardware Case A is significantly slower. Under QEMU user-mode
 * both cases run at similar instruction counts (no cache simulation).
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "../../common/benchmark.h"

#define ITERS      10000000
#define CACHE_LINE 64

/* ── Case A: adjacent counters (false sharing) ───────────────────────────── */
typedef struct {
    volatile uint64_t a;
    volatile uint64_t b;
} shared_adjacent_t;

/* ── Case B: padded counters (no false sharing) ──────────────────────────── */
typedef struct {
    volatile uint64_t a;
    char _pad[CACHE_LINE - sizeof(uint64_t)];
    volatile uint64_t b;
} shared_padded_t;

static shared_adjacent_t adj;
static shared_padded_t   pad;

static void *inc_adj_a(void *arg) {
    (void)arg;
    /* TODO: increment adj.a ITERS times */
    return NULL;
}

static void *inc_adj_b(void *arg) {
    (void)arg;
    /* TODO: increment adj.b ITERS times */
    return NULL;
}

static void *inc_pad_a(void *arg) {
    (void)arg;
    /* TODO: increment pad.a ITERS times */
    return NULL;
}

static void *inc_pad_b(void *arg) {
    (void)arg;
    /* TODO: increment pad.b ITERS times */
    return NULL;
}

static void run_adjacent(void) {
    adj.a = adj.b = 0;
    pthread_t t1, t2;
    pthread_create(&t1, NULL, inc_adj_a, NULL);
    pthread_create(&t2, NULL, inc_adj_b, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

static void run_padded(void) {
    pad.a = pad.b = 0;
    pthread_t t1, t2;
    pthread_create(&t1, NULL, inc_pad_a, NULL);
    pthread_create(&t2, NULL, inc_pad_b, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

int main(void) {
    BENCH_RUN("false_sharing_adjacent", 10, 2, { run_adjacent(); });
    BENCH_RUN("false_sharing_padded",   10, 2, { run_padded(); });
    return 0;
}
