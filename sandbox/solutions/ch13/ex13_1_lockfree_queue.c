/*
 * SOLUTION: Exercise 13.1 — Lock-Free SPSC Queue vs Mutex-Protected Queue
 * Chapter 13: Lock-Free Data Structures
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * Single-Producer / Single-Consumer (SPSC) lock-free queue:
 *   Only one thread writes (producer) and one thread reads (consumer) at a
 *   time. Because there is never concurrent write-write or read-write conflict
 *   on the *same* index, no CAS (compare-and-swap) instruction is needed. The
 *   only requirement is that index updates are visible across cores in the
 *   correct order — handled by C11 atomic_store/load with acquire-release
 *   semantics.
 *
 * Acquire-release protocol:
 *   Producer writes data[tail & MASK], then does a RELEASE store of tail++.
 *     Release ensures the data write is visible before the tail update.
 *   Consumer does an ACQUIRE load of tail to check emptiness.
 *     Acquire ensures it sees all data writes that happened before the
 *     corresponding release store.
 *   Symmetric rule applies to head (consumer releases, producer acquires).
 *
 * Mutex queue:
 *   A simple ring buffer protected by a single mutex. Every enqueue and
 *   dequeue acquires and releases the lock. On lightly contended hardware
 *   a mutex lock/unlock costs ~20–50 ns (lock prefix + cmpxchg on x86).
 *   With 100 000 messages this adds up. The SPSC avoids this cost entirely.
 *
 * Expected result: SPSC is typically 3–10× faster than the mutex queue.
 * Under QEMU the ratio is smaller (no real cache coherency traffic).
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

/*
 * STEP 1 — SPSC enqueue (producer only).
 *
 * STEP 1a: load head with ACQUIRE to see the consumer's latest read position.
 *   We need to know whether the queue is full. The consumer updates head
 *   with RELEASE after consuming; we ACQUIRE to synchronise with that.
 *
 * STEP 1b: full check.
 *   (tail - head) == QUEUE_CAP means every slot is occupied.
 *
 * STEP 1c: write the data, then store the new tail with RELEASE.
 *   The RELEASE store ensures the data write is globally visible before the
 *   consumer can see the updated tail.
 */
static int spsc_enqueue(spsc_queue_t *q, uint64_t val) {
    uint64_t tail = atomic_load_explicit(&q->tail, memory_order_relaxed);
    uint64_t head = atomic_load_explicit(&q->head, memory_order_acquire); /* STEP 1a */

    /* STEP 1b: full check */
    if (tail - head == QUEUE_CAP) return 0;

    /* STEP 1c: write data, then publish the new tail */
    q->data[tail & MASK] = val;
    atomic_store_explicit(&q->tail, tail + 1, memory_order_release);
    return 1;
}

/*
 * STEP 2 — SPSC dequeue (consumer only).
 *
 * STEP 2a: load tail with ACQUIRE to see the producer's latest writes.
 *
 * STEP 2b: empty check.
 *   head == tail means no elements are available.
 *
 * STEP 2c: read the data, then advance head with RELEASE.
 *   The RELEASE ensures the producer sees that the slot is free before it
 *   might try to overwrite it.
 */
static int spsc_dequeue(spsc_queue_t *q, uint64_t *val) {
    uint64_t head = atomic_load_explicit(&q->head, memory_order_relaxed);
    uint64_t tail = atomic_load_explicit(&q->tail, memory_order_acquire); /* STEP 2a */

    /* STEP 2b: empty check */
    if (head == tail) return 0;

    /* STEP 2c: read data, then publish the new head */
    *val = q->data[head & MASK];
    atomic_store_explicit(&q->head, head + 1, memory_order_release);
    return 1;
}

/* ── Mutex-protected queue ───────────────────────────────────────────────── */
typedef struct {
    uint64_t     data[QUEUE_CAP];
    uint32_t     head, tail;
    pthread_mutex_t lock;
} mutex_queue_t;

static mutex_queue_t mt_queue;

/*
 * STEP 3 — Mutex queue enqueue.
 *
 * Acquire the lock, check for space, write the element, release the lock.
 * The mutex provides all necessary memory ordering — no atomics needed.
 *
 * STEP 3a: full check uses the same (tail - head) == CAP logic.
 *   Inside the critical section head and tail are stable.
 */
static int mtq_enqueue(mutex_queue_t *q, uint64_t val) {
    pthread_mutex_lock(&q->lock);

    /* STEP 3a: full check inside the lock */
    if ((uint32_t)(q->tail - q->head) == QUEUE_CAP) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    q->data[q->tail & MASK] = val;
    q->tail++;
    pthread_mutex_unlock(&q->lock);
    return 1;
}

/*
 * STEP 4 — Mutex queue dequeue.
 *
 * Same pattern: lock, check empty, read, unlock.
 */
static int mtq_dequeue(mutex_queue_t *q, uint64_t *val) {
    pthread_mutex_lock(&q->lock);

    if (q->head == q->tail) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    *val = q->data[q->head & MASK];
    q->head++;
    pthread_mutex_unlock(&q->lock);
    return 1;
}

/* ── Thread functions ────────────────────────────────────────────────────── */
static volatile uint64_t lf_sink;
static volatile uint64_t mt_sink;

static void *lf_producer(void *arg) {
    (void)arg;
    for (uint64_t i = 0; i < MSG_COUNT; i++)
        while (!spsc_enqueue(&lf_queue, i)) {}
    return NULL;
}

static void *lf_consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < MSG_COUNT; i++) {
        uint64_t v;
        while (!spsc_dequeue(&lf_queue, &v)) {}
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

    BENCH_RUN("spsc_lockfree_100k", 5, 1, { run_lockfree(); });
    BENCH_RUN("mutex_queue_100k",   5, 1, { run_mutex(); });

    pthread_mutex_destroy(&mt_queue.lock);
    return 0;
}
