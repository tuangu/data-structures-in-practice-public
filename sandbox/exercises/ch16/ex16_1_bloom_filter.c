/*
 * Exercise 16.1: Bloom Filter
 * Chapter 16: Bloom Filters and Probabilistic Data Structures
 *
 * Implement a Bloom filter backed by a bit array.
 * Observe false-positive rate as a function of FILTER_BITS and NUM_HASH.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define FILTER_BITS  (1 << 16)     /* 64 KiB bit array */
#define FILTER_BYTES (FILTER_BITS / 8)
#define NUM_HASH     3
#define INSERT_COUNT 10000

static uint8_t filter[FILTER_BYTES];

/* TODO: implement k independent hash functions over key.
 * Hint: use FNV-1a with seed mixing: h = (h ^ seed) * 16777619 ^ key  */
static uint32_t bloom_hash(uint32_t key, int seed) {
    (void)key; (void)seed;
    return 0; /* TODO */
}

static void bloom_insert(uint32_t key) {
    for (int k = 0; k < NUM_HASH; k++) {
        uint32_t bit = bloom_hash(key, k) % FILTER_BITS;
        filter[bit / 8] |= (uint8_t)(1u << (bit % 8));
    }
}

static int bloom_query(uint32_t key) {
    for (int k = 0; k < NUM_HASH; k++) {
        uint32_t bit = bloom_hash(key, k) % FILTER_BITS;
        if (!(filter[bit / 8] & (uint8_t)(1u << (bit % 8)))) return 0;
    }
    return 1;
}

int main(void) {
    memset(filter, 0, sizeof(filter));
    for (uint32_t i = 0; i < INSERT_COUNT; i++) bloom_insert(i);

    /* False positive test: query keys that were NOT inserted */
    int fp = 0;
    for (uint32_t i = INSERT_COUNT; i < INSERT_COUNT * 2; i++)
        if (bloom_query(i)) fp++;

    printf("FILTER_BITS=%-6d  NUM_HASH=%d  inserted=%d  "
           "false_positives=%d  rate=%.2f%%\n",
           FILTER_BITS, NUM_HASH, INSERT_COUNT,
           fp, 100.0 * fp / INSERT_COUNT);
    printf("(All positives are false until TODO bloom_hash is implemented)\n");

    /* TODO: tune FILTER_BITS and NUM_HASH, observe rate change */

    volatile uint32_t sink = 0;
    BENCH_RUN("bloom_insert_10k", 200, 20, {
        memset(filter, 0, sizeof(filter));
        for (uint32_t i = 0; i < INSERT_COUNT; i++) bloom_insert(i);
    });
    BENCH_RUN("bloom_query_10k", 200, 20, {
        for (uint32_t i = 0; i < INSERT_COUNT; i++) sink ^= (uint32_t)bloom_query(i);
    });
    (void)sink;
    return 0;
}
