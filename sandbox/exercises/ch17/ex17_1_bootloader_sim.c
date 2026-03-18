/*
 * Exercise 17.1: Bootloader Data Structure Simulation
 * Chapter 17: Bootloader Data Structures
 *
 * Simulate device-tree parsing two ways:
 *   A) malloc-based linked lists (mimicking the slow 720 ms boot path)
 *   B) Static arena allocator (the optimised path)
 *
 * Compare counter deltas to see the allocator overhead difference.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define MAX_NODES    256
#define MAX_PROPS    16
#define ARENA_BYTES  (64 * 1024)

/* ── Data model ──────────────────────────────────────────────────────────── */
typedef struct dt_prop {
    char key[32];
    char val[32];
    struct dt_prop *next;
} dt_prop_t;

typedef struct dt_node {
    char name[32];
    dt_prop_t  *props;
    struct dt_node *next;
} dt_node_t;

/* ── malloc-based parse ──────────────────────────────────────────────────── */

/* TODO: malloc MAX_NODES dt_node_t structures, each with MAX_PROPS
 * dt_prop_t structures, link them into a list, and return the head.   */
static dt_node_t *linked_list_parse(void) {
    return NULL; /* TODO */
}

static void linked_list_free(dt_node_t *root) {
    /* TODO: walk the list, free every prop, then every node */
    (void)root;
}

/* ── Arena-based parse ───────────────────────────────────────────────────── */
static char   arena[ARENA_BYTES];
static size_t arena_offset;

/* TODO: bump allocator — advance arena_offset by sz (aligned to 8 bytes),
 * return pointer to the allocated region.                              */
static void *arena_alloc(size_t sz) {
    (void)sz;
    return NULL; /* TODO */
}

static void arena_reset(void) { arena_offset = 0; }

/* TODO: same structure as linked_list_parse but using arena_alloc.
 * No free required — the whole arena is discarded at once.            */
static dt_node_t *arena_parse(void) {
    return NULL; /* TODO */
}

int main(void) {
    BENCH_RUN("linked_list_parse", 100, 10, {
        dt_node_t *root = linked_list_parse();
        linked_list_free(root);
    });

    BENCH_RUN("arena_parse", 100, 10, {
        arena_reset();
        (void)arena_parse();
    });

    return 0;
}
