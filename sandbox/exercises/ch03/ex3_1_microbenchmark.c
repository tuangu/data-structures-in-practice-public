/*
 * Exercise 3.1: Microbenchmark Framework Self-Test
 * Chapter 3: Benchmarking and Profiling
 *
 * Use the benchmark framework to measure the overhead of read_counter()
 * itself, and demonstrate warmup / statistical stability.
 *
 * Under QEMU rdcycle returns instruction count so the empty-loop counter
 * grows linearly with INNER_OPS — a good sanity check.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define INNER_OPS 1000

int main(void) {
    volatile uint64_t sink = 0;

    /* Baseline: measure a tight increment loop */
    BENCH_RUN("empty_loop_1000", 200, 20, {
        for (int i = 0; i < INNER_OPS; i++) sink++;
    });

    /* Measure read_counter() overhead by calling it twice back-to-back */
    BENCH_RUN("read_counter_overhead", 200, 20, {
        uint64_t t0 = read_counter();
        uint64_t t1 = read_counter();
        sink ^= (t1 - t0);
    });

    /* TODO: add a memory-bound workload (e.g. linked-list traversal) and
     * compare against the loop above. How many instructions does a cache
     * miss cost under QEMU vs real hardware?                              */

    (void)sink;
    return 0;
}
