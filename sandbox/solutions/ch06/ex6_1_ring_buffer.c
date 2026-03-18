/*
 * SOLUTION: Exercise 6.1 — Ring Buffer (Circular Queue)
 * Chapter 6: Queues and Ring Buffers
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A ring buffer stores elements in a fixed-size array, reusing slots once they
 * are consumed. Two indices — head (read) and tail (write) — advance through
 * the array and wrap around at the capacity boundary.
 *
 * Power-of-2 optimisation:
 *   Instead of  index = index % CAP  (division — slow on embedded CPUs),
 *   we use       index & MASK        (bitwise AND — single cycle).
 *   This works only when CAP is a power of 2, because then
 *   MASK = CAP - 1 is all 1-bits in the lower positions.
 *
 * Full/empty detection:
 *   We keep head and tail as *monotonically increasing* counters (never
 *   wrapped). The buffer is:
 *     empty when tail == head
 *     full  when tail - head == CAP
 *   Actual slot = counter & MASK.
 *
 * This design avoids the classic "one slot wasted" trick and makes the
 * full/empty check correct without extra state.
 */
#include <stdio.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define CAP  (1 << 14)   /* 16 384 — must be power of 2 */
#define MASK (CAP - 1)
#define OPS  10000

typedef struct {
    uint64_t data[CAP];
    uint32_t head;   /* consumer reads from data[head & MASK], then head++ */
    uint32_t tail;   /* producer writes to data[tail & MASK], then tail++  */
} ring_buf_t;

static ring_buf_t rb;

/*
 * STEP 1 — Enqueue (producer side).
 *
 * STEP 1a: check for full.
 *   (tail - head) gives the number of elements currently in the buffer.
 *   If it equals CAP, the buffer is full. We cast to uint32_t deliberately:
 *   tail and head are uint32_t, so subtraction wraps at 2^32, and the
 *   comparison still works correctly even after a 32-bit overflow because
 *   we only ever have at most CAP < 2^32 outstanding elements.
 *
 * STEP 1b: write to the slot at tail & MASK, then increment tail.
 *   The bitmask converts the monotonic counter to a slot index within [0, CAP).
 */
static int rb_enqueue(ring_buf_t *r, uint64_t val) {
    /* STEP 1a: full check — (tail - head) == CAP means no room */
    if ((uint32_t)(r->tail - r->head) == CAP)
        return 0;  /* full */

    /* STEP 1b: write to the next available slot, advance tail */
    r->data[r->tail & MASK] = val;
    r->tail++;
    return 1;
}

/*
 * STEP 2 — Dequeue (consumer side).
 *
 * STEP 2a: check for empty.
 *   head == tail means no elements are available.
 *
 * STEP 2b: read from data[head & MASK], store into *val, advance head.
 *   The order matters: read before incrementing, otherwise the slot might
 *   be overwritten by a concurrent producer before we read it (in a
 *   multi-threaded scenario; not an issue here but good habit).
 */
static int rb_dequeue(ring_buf_t *r, uint64_t *val) {
    /* STEP 2a: empty check */
    if (r->head == r->tail)
        return 0;  /* empty */

    /* STEP 2b: read from the oldest slot, advance head */
    *val = r->data[r->head & MASK];
    r->head++;
    return 1;
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
