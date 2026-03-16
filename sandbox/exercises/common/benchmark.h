#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Cycle / time reading ─────────────────────────────────────────────────── */

/*
 * read_counter() returns a monotonically increasing 64-bit counter.
 *
 * On RISC-V:   rdcycle CSR — under QEMU user-mode this returns the
 *              *instruction count*, not real wall-clock cycles. Values are
 *              still useful for relative comparisons between two code paths
 *              measured back-to-back in the same run.
 *
 * On x86-64:   rdtsc — the time-stamp counter (invariant TSC assumed).
 *
 * Fallback:    clock_gettime(CLOCK_MONOTONIC) converted to nanoseconds.
 *              Used on any architecture not explicitly handled above.
 */
static inline uint64_t read_counter(void) {
#if defined(__riscv)
    uint64_t c;
    __asm__ volatile ("rdcycle %0" : "=r"(c));
    return c;
#elif defined(__x86_64__) || defined(_M_X64)
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

/* ── Statistics ───────────────────────────────────────────────────────────── */

typedef struct {
    uint64_t min;
    uint64_t max;
    double   mean;
    double   stddev;
} bench_stats_t;

/*
 * Compute statistics over an array of raw counter deltas.
 * samples : array of (stop - start) values collected per iteration
 * n       : number of samples
 * out     : filled in by this function
 */
void bench_stats(const uint64_t *samples, size_t n, bench_stats_t *out);

/*
 * Print one result line to stdout.
 * Declared here so exercises can call it directly if not using BENCH_RUN.
 */
void bench_print_result(const char *label, const bench_stats_t *st);

/* ── Runner macro ─────────────────────────────────────────────────────────── */

/*
 * BENCH_RUN(label, iterations, warmup, body)
 *
 * Convenience macro. Runs `body` for `warmup` iterations (results
 * discarded), then for `iterations` iterations collecting counter deltas,
 * then prints a summary line to stdout.
 *
 * `body` is a C statement or block. It must not contain `return`.
 *
 * Example:
 *   BENCH_RUN("array_sum", 1000, 50, {
 *       volatile int s = 0;
 *       for (int i = 0; i < N; i++) s += arr[i];
 *   });
 *
 * NOTE: `iterations` must be a compile-time constant (VLA on the stack).
 */
#define BENCH_RUN(label, iterations, warmup, body)                          \
    do {                                                                    \
        uint64_t _samples[(iterations)];                                    \
        /* warmup */                                                        \
        for (int _w = 0; _w < (warmup); _w++) { body; }                    \
        /* measure */                                                       \
        for (int _i = 0; _i < (iterations); _i++) {                        \
            uint64_t _t0 = read_counter();                                  \
            { body; }                                                       \
            uint64_t _t1 = read_counter();                                  \
            _samples[_i] = _t1 - _t0;                                      \
        }                                                                   \
        bench_stats_t _st;                                                  \
        bench_stats(_samples, (iterations), &_st);                         \
        bench_print_result((label), &_st);                                  \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* BENCHMARK_H */
