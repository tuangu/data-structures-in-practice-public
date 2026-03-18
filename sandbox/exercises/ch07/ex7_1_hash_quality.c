/*
 * Exercise 7.1: Hash Function Quality Comparison
 * Chapter 7: Hash Tables
 *
 * Compare three hash functions over 10 000 string keys:
 *   A) Naive sum of bytes
 *   B) FNV-1a (32-bit)
 *   C) Murmur3 finalizer mix
 *
 * Metric: bucket occupancy variance across TABLE_SIZE buckets.
 * Lower variance = better uniformity = fewer collisions.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "../../common/benchmark.h"

#define TABLE_SIZE  1024
#define NUM_KEYS    10000
#define KEY_LEN     16

static char keys[NUM_KEYS][KEY_LEN];

static void setup(void) {
    /* Generate pseudo-random fixed-length keys */
    uint32_t state = 0xdeadbeef;
    for (int i = 0; i < NUM_KEYS; i++) {
        for (int j = 0; j < KEY_LEN - 1; j++) {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            keys[i][j] = 'a' + (char)(state % 26);
        }
        keys[i][KEY_LEN - 1] = '\0';
    }
}

/* ── Hash functions ──────────────────────────────────────────────────────── */

/* TODO: implement naive byte-sum hash */
static uint32_t hash_sum(const char *key) {
    (void)key;
    return 0; /* TODO */
}

/* TODO: implement FNV-1a 32-bit hash */
static uint32_t hash_fnv1a(const char *key) {
    (void)key;
    return 0; /* TODO */
}

/* TODO: implement Murmur3 32-bit finalizer mix */
static uint32_t hash_murmur3(const char *key) {
    (void)key;
    return 0; /* TODO */
}

/* ── Measure variance ────────────────────────────────────────────────────── */
typedef uint32_t (*hash_fn_t)(const char *);

static double bucket_variance(hash_fn_t fn) {
    int counts[TABLE_SIZE] = {0};
    for (int i = 0; i < NUM_KEYS; i++)
        counts[fn(keys[i]) % TABLE_SIZE]++;
    double mean = (double)NUM_KEYS / TABLE_SIZE;
    double var = 0.0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        double d = counts[i] - mean;
        var += d * d;
    }
    return var / TABLE_SIZE;
}

int main(void) {
    setup();

    printf("%-20s  variance\n", "hash_function");
    printf("%-20s  %.2f\n", "sum_of_bytes",  bucket_variance(hash_sum));
    printf("%-20s  %.2f\n", "fnv1a_32",      bucket_variance(hash_fnv1a));
    printf("%-20s  %.2f\n", "murmur3_mix",   bucket_variance(hash_murmur3));

    /* Also benchmark speed */
    volatile uint32_t sink = 0;
    BENCH_RUN("hash_sum_10k",    200, 20, {
        for (int i = 0; i < NUM_KEYS; i++) sink ^= hash_sum(keys[i]);
    });
    BENCH_RUN("hash_fnv1a_10k",  200, 20, {
        for (int i = 0; i < NUM_KEYS; i++) sink ^= hash_fnv1a(keys[i]);
    });
    BENCH_RUN("hash_murmur_10k", 200, 20, {
        for (int i = 0; i < NUM_KEYS; i++) sink ^= hash_murmur3(keys[i]);
    });
    (void)sink;
    return 0;
}
