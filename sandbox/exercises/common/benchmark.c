#include "benchmark.h"
#include <math.h>
#include <stdio.h>
#include <stddef.h>

void bench_stats(const uint64_t *samples, size_t n, bench_stats_t *out) {
    if (n == 0) {
        *out = (bench_stats_t){0, 0, 0.0, 0.0};
        return;
    }

    uint64_t min = samples[0], max = samples[0];
    double   sum = 0.0;

    for (size_t i = 0; i < n; i++) {
        if (samples[i] < min) min = samples[i];
        if (samples[i] > max) max = samples[i];
        sum += (double)samples[i];
    }

    double mean = sum / (double)n;

    double var = 0.0;
    for (size_t i = 0; i < n; i++) {
        double d = (double)samples[i] - mean;
        var += d * d;
    }
    var /= (double)n;   /* population stddev */

    out->min    = min;
    out->max    = max;
    out->mean   = mean;
    out->stddev = sqrt(var);
}

void bench_print_result(const char *label, const bench_stats_t *st) {
    printf("%-40s  mean=%10.0f  stddev=%8.0f  min=%10llu  max=%10llu\n",
           label,
           st->mean,
           st->stddev,
           (unsigned long long)st->min,
           (unsigned long long)st->max);
}
