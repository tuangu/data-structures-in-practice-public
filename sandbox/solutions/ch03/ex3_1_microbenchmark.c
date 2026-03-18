/*
 * SOLUTION: Exercise 3.1 — Microbenchmark Framework Self-Test
 * Chapter 3: Benchmarking and Profiling
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * This exercise has two parts:
 *
 *   Part A (provided): Measure a tight increment loop and read_counter()
 *     overhead to understand the baseline cost of the benchmark harness.
 *
 *   Part B (TODO, implemented here): Add a memory-bound workload — a
 *     linked-list traversal over a large list that doesn't fit in cache —
 *     and compare its instruction/cycle count against the tight loop.
 *
 * The comparison demonstrates the fundamental lesson of Chapter 3:
 *   "Memory-bound code costs far more than compute-bound code."
 *
 * Under QEMU: both cases increment the instruction counter at a similar rate
 * per operation (no cache simulation), so the ratio will be close to 1.
 *
 * On real hardware: the linked-list traverse causes one cache miss per node
 * (~100–200 cycles on x86-64, ~80–150 cycles on RISC-V). The tight loop runs
 * at ~1 cycle/iteration. The ratio reveals the true cost of a cache miss.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define INNER_OPS   1000
#define LIST_NODES  4096   /* > L1 + L2 on most embedded targets */

/*
 * STEP 1 — Build a linked list for the memory-bound workload.
 *
 * Each node is allocated with malloc(), which (on a live heap) places nodes
 * at scattered addresses. The traversal pattern is therefore non-sequential
 * and unpredictable — ideal for stressing the cache.
 *
 * STEP 1a: node layout
 *   We include a `val` field to prevent the compiler from optimising away
 *   the list entirely. The pointer + value pair is 16 bytes — smaller than
 *   a cache line, so consecutive nodes are NOT guaranteed to share a line.
 */
typedef struct node { int val; struct node *next; } node_t;

static node_t *list_head;

static void build_list(void) {
    list_head = NULL;
    /* STEP 1b: prepend nodes so the list order is reverse-insertion order.
     * malloc() typically returns blocks from a free list, so adjacent
     * allocations may or may not be adjacent in memory. */
    for (int i = 0; i < LIST_NODES; i++) {
        node_t *n = malloc(sizeof(*n));
        if (!n) { perror("malloc"); exit(1); }
        n->val  = i;
        n->next = list_head;
        list_head = n;
    }
}

int main(void) {
    volatile uint64_t sink = 0;

    /* Part A: provided — baseline tight loop */
    BENCH_RUN("empty_loop_1000", 200, 20, {
        for (int i = 0; i < INNER_OPS; i++) sink++;
    });

    /* Part A: provided — read_counter() call overhead */
    BENCH_RUN("read_counter_overhead", 200, 20, {
        uint64_t t0 = read_counter();
        uint64_t t1 = read_counter();
        sink ^= (t1 - t0);
    });

    /*
     * STEP 2 — Memory-bound workload: linked-list traversal.
     *
     * We build the list once outside BENCH_RUN so allocation cost is not
     * included in the measurement. Only the pointer-chasing traversal is
     * timed.
     *
     * STEP 2a: why traverse the *whole* list per iteration?
     *   LIST_NODES = 4096 nodes × 16 bytes = 64 KB. Most L1 caches are 32–64
     *   KB, so the list does not fit in L1 (and often not in L2 either). Each
     *   node access is therefore a cache miss in steady state.
     *
     * STEP 2b: the volatile sum forces the CPU to actually load node->val.
     *   Without it the compiler may transform the traversal into a no-op
     *   since the result is unused.
     */
    build_list();

    BENCH_RUN("linked_list_traverse_4k", 50, 10, {
        int64_t sum = 0;
        for (node_t *n = list_head; n; n = n->next)
            sum += n->val;          /* dependent load chain — no prefetch help */
        sink ^= (uint64_t)sum;
    });

    /*
     * STEP 3 — Interpretation guide (printed to help the learner).
     *
     * Compute the approximate cost per element for each workload so the
     * learner can see the ratio directly in the output.
     */
    printf("\n--- Interpretation ---\n");
    printf("empty_loop:     ~1 instruction/iteration (register only)\n");
    printf("list_traverse:  ~1+ cache miss per node (%d nodes)\n", LIST_NODES);
    printf("On real hardware the list traverse is 50-200x slower per element.\n");
    printf("Under QEMU (instruction counting) the ratio is much smaller.\n");

    (void)sink;
    return 0;
}
