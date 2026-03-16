/*
 * Exercise 13.1: Lock-Free SPSC Queue vs Mutex-Protected Queue
 * Chapter 13: Lock-Free Data Structures
 *
 * Implement a single-producer / single-consumer lock-free ring buffer
 * (SPSC — no ABA problem, simpler starting point than MPMC).
 * Compare throughput with a mutex-protected queue.
 *
 * SPSC ring: producer increments tail, consumer increments head.
 * No CAS needed because only one writer and one reader at a time.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include "../../common/benchmark.h"

#define QUEUE_CAP  (1 << 12)   /* must be power of 2 */
#define MASK       (QUEUE_CAP - 1)
#define MSG_COUNT  100000

/* ── Lock-free SPSC ring ──────────────────────────────────────────────────── */
typedef struct {
    _Atomic uint64_t head;
    _Atomic uint64_t tail;
    uint64_t         data[QUEUE_CAP];
} spsc_queue_t;

static spsc_queue_t lf_queue;

/* TODO: enqueue val; return 1 on success, 0 if full */
static int spsc_enqueue(spsc_queue_t *q, uint64_t val) {
    (void)q; (void)val;
    return 0; /* TODO */
}

/* TODO: dequeue into *val; return 1 on success, 0 if empty */
static int spsc_dequeue(spsc_queue_t *q, uint64_t *val) {
    (void)q; (void)val;
    return 0; /* TODO */
}

/* ── Mutex-protected queue (simple ring) ─────────────────────────────────── */
typedef struct {
    uint64_t     data[QUEUE_CAP];
    uint32_t     head, tail;
    pthread_mutex_t lock;
} mutex_queue_t;

static mutex_queue_t mt_queue;

static int mtq_enqueue(mutex_queue_t *q, uint64_t val) {
    /* TODO */
    (void)q; (void)val;
    return 0;
}

static int mtq_dequeue(mutex_queue_t *q, uint64_t *val) {
    /* TODO */
    (void)q; (void)val;
    return 0;
}

/* ── Thread functions ────────────────────────────────────────────────────── */
static volatile uint64_t lf_sink;
static volatile uint64_t mt_sink;

static void *lf_producer(void *arg) {
    (void)arg;
    for (uint64_t i = 0; i < MSG_COUNT; i++)
        while (!spsc_enqueue(&lf_queue, i)) {}   /* spin */
    return NULL;
}

static void *lf_consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < MSG_COUNT; i++) {
        uint64_t v;
        while (!spsc_dequeue(&lf_queue, &v)) {}  /* spin */
        lf_sink ^= v;
    }
    return NULL;
}

static void *mt_producer(void *arg) {
    (void)arg;
    for (uint64_t i = 0; i < MSG_COUNT; i++)
        while (!mtq_enqueue(&mt_queue, i)) {}
    return NULL;
}

static void *mt_consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < MSG_COUNT; i++) {
        uint64_t v;
        while (!mtq_dequeue(&mt_queue, &v)) {}
        mt_sink ^= v;
    }
    return NULL;
}

static void run_lockfree(void) {
    atomic_store(&lf_queue.head, 0);
    atomic_store(&lf_queue.tail, 0);
    pthread_t prod, cons;
    pthread_create(&prod, NULL, lf_producer, NULL);
    pthread_create(&cons, NULL, lf_consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
}

static void run_mutex(void) {
    mt_queue.head = mt_queue.tail = 0;
    pthread_t prod, cons;
    pthread_create(&prod, NULL, mt_producer, NULL);
    pthread_create(&cons, NULL, mt_consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
}

int main(void) {
    pthread_mutex_init(&mt_queue.lock, NULL);

    printf("Lock-free queue exercise (implement TODOs to see real results)\n");

    BENCH_RUN("spsc_lockfree_100k", 5, 1, { run_lockfree(); });
    BENCH_RUN("mutex_queue_100k",   5, 1, { run_mutex(); });

    pthread_mutex_destroy(&mt_queue.lock);
    return 0;
}
