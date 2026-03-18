/*
 * SOLUTION: Exercise 2.2 — False Sharing Between Threads
 * Chapter 2: Memory Hierarchy
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * False sharing occurs when two threads write to *different* variables that
 * happen to live on the *same* cache line. Even though neither thread reads
 * the other's variable, the coherency protocol treats the entire cache line
 * as the unit of ownership. Every time one thread writes, the line is
 * invalidated in the other core's cache, forcing a round-trip to get the
 * latest version before the next write — even though the data it actually
 * needs hasn't changed.
 *
 * Case A (adjacent): adj.a and adj.b are 8 bytes apart — they share a 64-byte
 *   cache line. Thread 1 writes adj.a, Thread 2 writes adj.b. Each write
 *   invalidates the line in the other core. Effectively serialised.
 *
 * Case B (padded): pad.a and pad.b are separated by 56 bytes of padding so
 *   they land on different cache lines. The two threads never contend for the
 *   same line. Both run at full speed concurrently.
 *
 * NOTE: Under QEMU user-mode there is no cache simulation; both cases execute
 * a similar number of instructions. Run on real SMP hardware (x86-64 with
 * multiple cores) to observe the 3–10× slowdown in Case A.
 */
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "../common/benchmark.h"

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

/*
 * STEP 1 — Adjacent counter thread functions.
 *
 * Each function simply increments its counter ITERS times using a plain
 * pre-increment (++). No mutex is needed because each thread writes to its
 * own counter — the race condition is in the *cache coherency protocol*, not
 * in the data values themselves.
 *
 * STEP 1a: why volatile?
 *   Without volatile the compiler may keep the counter in a register and
 *   write it back only once, defeating the point of the exercise. volatile
 *   forces every iteration to be a real memory store, making the false-sharing
 *   penalty visible.
 */
static void *inc_adj_a(void *arg) {
    (void)arg;
    /* STEP 1b: increment adj.a — each store may invalidate adj.b's cache line */
    for (int i = 0; i < ITERS; i++) adj.a++;
    return NULL;
}

static void *inc_adj_b(void *arg) {
    (void)arg;
    /* STEP 1c: increment adj.b — shares a cache line with adj.a */
    for (int i = 0; i < ITERS; i++) adj.b++;
    return NULL;
}

/*
 * STEP 2 — Padded counter thread functions.
 *
 * Same logic, but pad.a and pad.b live on separate cache lines because of
 * the _pad[56] field between them. Each thread's counter fits entirely within
 * its own private cache line. Writes by one thread do not invalidate the
 * other's line.
 */
static void *inc_pad_a(void *arg) {
    (void)arg;
    /* STEP 2a: pad.a is on a dedicated cache line — no coherency traffic for pad.b */
    for (int i = 0; i < ITERS; i++) pad.a++;
    return NULL;
}

static void *inc_pad_b(void *arg) {
    (void)arg;
    /* STEP 2b: pad.b is on a dedicated cache line — no coherency traffic for pad.a */
    for (int i = 0; i < ITERS; i++) pad.b++;
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
