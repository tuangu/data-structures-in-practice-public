/*
 * SOLUTION: Exercise 1.1 — Hash Table vs Binary Search
 * Chapter 1: The Performance Gap
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * The exercise models a common firmware lookup problem: 500 device config
 * entries, 10 000 lookups. We compare two approaches:
 *
 *   A) Binary search on a sorted array — O(log n) comparisons, excellent
 *      cache behaviour because the array is contiguous.
 *
 *   B) Hash table with separate chaining — O(1) average, but each bucket
 *      chain node was malloc'd individually, so pointer chasing is required.
 *
 * Surprising result: on a real CPU with a hot cache both are fast, but the
 * hash table wins when the working set is larger than a cache level. Under
 * QEMU rdcycle counts instructions, so instruction count drives the result.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define NUM_ENTRIES   500
#define TABLE_BUCKETS 1024
#define NUM_LOOKUPS   10000

typedef struct { uint32_t device_id; uint32_t config_val; } device_cfg_t;

/* ── Sorted array for binary search ─────────────────────────────────────── */
static device_cfg_t sorted_table[NUM_ENTRIES];

static int cmp_cfg(const void *a, const void *b) {
    uint32_t ia = ((const device_cfg_t *)a)->device_id;
    uint32_t ib = ((const device_cfg_t *)b)->device_id;
    return (ia > ib) - (ia < ib);
}

/*
 * STEP 1 — Binary search on sorted_table[].
 *
 * Binary search works by maintaining an [lo, hi) half-open interval and
 * repeatedly halving it. Key points:
 *
 *   - mid = lo + (hi - lo) / 2  avoids the classic (lo+hi)/2 overflow.
 *   - We return -1 (cast to int32_t) as a sentinel for "not found".
 *   - With 500 entries log2(500) ≈ 9, so at most 9 comparisons per lookup.
 *
 * Cache behaviour: sorted_table fits in ~4 KB (500 × 8 bytes). On most CPUs
 * it stays in L1 cache after the first pass, making subsequent lookups very
 * fast despite O(log n) comparisons.
 */
static int32_t bsearch_lookup(uint32_t id) {
    int lo = 0, hi = NUM_ENTRIES;

    /* STEP 1a: standard half-open interval binary search */
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (sorted_table[mid].device_id == id)
            return (int32_t)sorted_table[mid].config_val;  /* found */
        else if (sorted_table[mid].device_id < id)
            lo = mid + 1;   /* key is in the upper half */
        else
            hi = mid;       /* key is in the lower half */
    }

    return -1;  /* not found */
}

/* ── Hash table with chaining ────────────────────────────────────────────── */
typedef struct bucket_node {
    uint32_t device_id;
    uint32_t config_val;
    struct bucket_node *next;
} bucket_node_t;

static bucket_node_t *hash_table[TABLE_BUCKETS];

static uint32_t hash_fn(uint32_t id) { return id % TABLE_BUCKETS; }

/*
 * STEP 2 — Hash table lookup with separate chaining.
 *
 *   - Compute the bucket index with hash_fn (already provided).
 *   - Walk the singly-linked chain at that bucket comparing device_id.
 *   - Return config_val on match, -1 if the chain is exhausted.
 *
 * Cache behaviour: each bucket_node_t was independently malloc'd. The nodes
 * for a single bucket are unlikely to be adjacent in memory, so traversing
 * a long chain causes cache misses. With TABLE_BUCKETS=1024 and 500 entries
 * the average chain length is < 1, so in practice most lookups hit the
 * first node — almost no pointer chasing occurs.
 *
 * This is why load factor matters: keep it below ~0.7 to maintain O(1)
 * average and minimal cache pressure.
 */
static int32_t hash_lookup(uint32_t id) {
    /* STEP 2a: map the key to a bucket */
    uint32_t bucket = hash_fn(id);

    /* STEP 2b: walk the chain until we find the key or reach the end */
    for (bucket_node_t *node = hash_table[bucket]; node; node = node->next) {
        if (node->device_id == id)
            return (int32_t)node->config_val;
    }

    return -1;  /* not found */
}

/* ── Setup ───────────────────────────────────────────────────────────────── */
static uint32_t lookup_ids[NUM_LOOKUPS];

static void setup(void) {
    for (int i = 0; i < NUM_ENTRIES; i++) {
        sorted_table[i].device_id  = (uint32_t)(i * 7 + 3);
        sorted_table[i].config_val = (uint32_t)i;
    }
    qsort(sorted_table, NUM_ENTRIES, sizeof(device_cfg_t), cmp_cfg);

    for (int i = 0; i < NUM_ENTRIES; i++) {
        bucket_node_t *node = malloc(sizeof(*node));
        if (!node) { perror("malloc"); exit(1); }
        node->device_id  = sorted_table[i].device_id;
        node->config_val = sorted_table[i].config_val;
        uint32_t h = hash_fn(node->device_id);
        node->next   = hash_table[h];
        hash_table[h] = node;
    }

    for (int i = 0; i < NUM_LOOKUPS; i++)
        lookup_ids[i] = sorted_table[i % NUM_ENTRIES].device_id;
}

int main(void) {
    setup();

    volatile int32_t sink = 0;

    BENCH_RUN("bsearch_lookup", 500, 50, {
        for (int i = 0; i < NUM_LOOKUPS; i++)
            sink ^= bsearch_lookup(lookup_ids[i]);
    });

    BENCH_RUN("hash_lookup", 500, 50, {
        for (int i = 0; i < NUM_LOOKUPS; i++)
            sink ^= hash_lookup(lookup_ids[i]);
    });

    (void)sink;
    return 0;
}
