/*
 * Exercise 1.1: Hash Table vs Binary Search
 * Chapter 1: The Performance Gap
 *
 * Reproduce the opening story: 500 device config entries, 10 000 lookups.
 * Measure relative performance of hash lookup vs binary search.
 *
 * NOTE: Under QEMU rdcycle returns instruction count — still useful for
 * relative A-vs-B comparison within the same run.
 *
 * Build:
 *   make ARCH=riscv64   (inside Docker or with cross-tools installed)
 *   make ARCH=x86_64
 * Run under QEMU:
 *   qemu-riscv64-static -L /usr/riscv64-linux-gnu build/riscv64/ch01/ex1_1_hash_vs_bsearch
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../common/benchmark.h"

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

/* TODO: implement binary search; return config_val or -1 if not found */
static int32_t bsearch_lookup(uint32_t id) {
    (void)id;
    return -1; /* TODO */
}

/* ── Hash table with chaining ────────────────────────────────────────────── */
typedef struct bucket_node {
    uint32_t device_id;
    uint32_t config_val;
    struct bucket_node *next;
} bucket_node_t;

static bucket_node_t *hash_table[TABLE_BUCKETS];

static uint32_t hash_fn(uint32_t id) { return id % TABLE_BUCKETS; }

/* TODO: implement hash lookup; return config_val or -1 if not found */
static int32_t hash_lookup(uint32_t id) {
    (void)id;
    return -1; /* TODO */
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
