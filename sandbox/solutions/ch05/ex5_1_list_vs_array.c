/*
 * SOLUTION: Exercise 5.1 — Linked List Stack vs Array Stack
 * Chapter 5: Linked Lists
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A stack is a Last-In-First-Out structure. Both implementations are
 * semantically equivalent, but their memory access patterns differ greatly:
 *
 *   Array stack:
 *     Stores elements in a pre-allocated contiguous block. Push = write to
 *     arr[++top]; pop = read arr[top--]. No allocation per operation.
 *     All accesses are within a small, hot region of memory — L1 cache hit
 *     rate is near 100%.
 *
 *   Linked-list stack:
 *     Each push calls malloc() for a new node. Each pop calls free().
 *     malloc/free themselves have non-trivial overhead (lock, free-list
 *     search, bookkeeping). Additionally, nodes scattered across the heap
 *     produce cache misses during pop traversal.
 *
 * Expected result: array stack is typically 5–20× faster per push/pop pair.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define N 10000

/* ── Array stack ─────────────────────────────────────────────────────────── */
static int arr_stack[N];
static int arr_top = -1;

/*
 * STEP 1 — Array stack push.
 *
 * Pre-increment arr_top then store val at that index. The -1 sentinel means
 * "empty", so the first push writes to index 0. No bounds check is included
 * because the benchmark always pushes exactly N elements and the array has
 * capacity N.
 *
 * Cache behaviour: arr_stack[arr_top] is very likely in L1 cache because we
 * just accessed the adjacent element one iteration ago.
 */
static void arr_push(int val) {
    arr_stack[++arr_top] = val;  /* STEP 1a: advance top, then write */
}

/*
 * STEP 2 — Array stack pop.
 *
 * Read the element at arr_top then decrement. Post-decrement of the index
 * keeps the reads in the right order. Returns 0 on empty stack (not reached
 * in this benchmark, but safe to return a default value).
 */
static int arr_pop(void) {
    if (arr_top < 0) return 0;          /* empty guard */
    return arr_stack[arr_top--];        /* STEP 2a: read, then retract top */
}

/* ── Linked-list stack ───────────────────────────────────────────────────── */
typedef struct node { int val; struct node *next; } node_t;
static node_t *list_head = NULL;

/*
 * STEP 3 — Linked-list stack push.
 *
 * Allocate a new node with malloc, set its value, and prepend it at the head
 * of the list. Prepend is O(1) and avoids traversal.
 *
 * STEP 3a: malloc overhead
 *   A typical glibc malloc call acquires a lock (even in single-threaded
 *   code), searches a size-class free list, and updates bookkeeping metadata.
 *   This alone costs hundreds of nanoseconds per call.
 *
 * STEP 3b: the new node's address is wherever the allocator decides to put
 *   it — not necessarily adjacent to the previous node. Over many pushes the
 *   nodes spread across the heap.
 */
static void list_push(int val) {
    node_t *n = malloc(sizeof(*n));     /* STEP 3c: per-push allocation */
    if (!n) { perror("malloc"); exit(1); }
    n->val  = val;
    n->next = list_head;                /* STEP 3d: prepend at head — O(1) */
    list_head = n;
}

/*
 * STEP 4 — Linked-list stack pop.
 *
 * Save the head pointer, advance list_head to the next node, free the old
 * head, and return its value. Steps must be in this order to avoid a
 * use-after-free.
 *
 * STEP 4a: free() has similar overhead to malloc() — lock + bookkeeping.
 * STEP 4b: reading node->val and node->next may cause a cache miss if the
 *   node was allocated long ago and has been evicted from cache.
 */
static int list_pop(void) {
    if (!list_head) return 0;           /* empty guard */
    node_t *old = list_head;            /* STEP 4c: save before advancing */
    list_head   = old->next;            /* STEP 4d: advance head */
    int val     = old->val;             /* STEP 4e: read value before free */
    free(old);                          /* STEP 4f: release memory */
    return val;
}

/* ── Workloads ───────────────────────────────────────────────────────────── */
static void workload_arr(void) {
    for (int i = 0; i < N; i++) arr_push(i);
    for (int i = 0; i < N; i++) (void)arr_pop();
}

static void workload_list(void) {
    for (int i = 0; i < N; i++) list_push(i);
    for (int i = 0; i < N; i++) (void)list_pop();
}

int main(void) {
    BENCH_RUN("array_stack_push_pop",  200, 20, { workload_arr(); });
    BENCH_RUN("list_stack_push_pop",   200, 20, { workload_list(); });
    return 0;
}
