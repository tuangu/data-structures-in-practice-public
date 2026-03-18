/*
 * Exercise 6.1: Ring Buffer (Circular Queue)
 * Chapter 6: Queues and Ring Buffers
 *
 * Implement a ring buffer with power-of-2 capacity using a bitmask for
 * the modulo operation. Measure enqueue + dequeue of 10 000 items.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define CAP  (1 << 14)   /* 16 384 — must be power of 2 */
#define MASK (CAP - 1)
#define OPS  10000

typedef struct {
    uint64_t data[CAP];
    uint32_t head;
    uint32_t tail;
} ring_buf_t;

static ring_buf_t rb;

/* TODO: enqueue val; return 1 on success, 0 if full */
static int rb_enqueue(ring_buf_t *r, uint64_t val) {
    (void)r; (void)val;
    return 0; /* TODO */
}

/* TODO: dequeue into *val; return 1 on success, 0 if empty */
static int rb_dequeue(ring_buf_t *r, uint64_t *val) {
    (void)r; (void)val;
    return 0; /* TODO */
}

static void workload(void) {
    rb.head = rb.tail = 0;
    for (uint64_t i = 0; i < OPS; i++) rb_enqueue(&rb, i);
    uint64_t sink = 0;
    uint64_t v;
    for (int i = 0; i < OPS; i++) if (rb_dequeue(&rb, &v)) sink ^= v;
    (void)sink;
}

int main(void) {
    BENCH_RUN("ring_buffer_enqueue_dequeue", 200, 20, { workload(); });
    return 0;
}
