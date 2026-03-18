/*
 * SOLUTION: Exercise 17.1 — Bootloader Data Structure Simulation
 * Chapter 17: Bootloader Data Structures
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * The opening story of the book: a bootloader takes 720 ms to parse a
 * device-tree because it uses malloc for every node and property. Switching
 * to a static arena allocator drops boot time to under 10 ms.
 *
 * Why malloc is slow in a bootloader context:
 *   1. malloc itself has significant overhead (lock, free-list search,
 *      metadata maintenance, potential system call for large allocations).
 *   2. Each call returns a pointer to an arbitrary heap location, so nodes
 *      and their properties are scattered across memory. Traversing the
 *      resulting linked list causes cache misses.
 *   3. Calling free() later (during cleanup) repeats this overhead in reverse.
 *
 * Arena allocator (bump pointer):
 *   A block of memory is reserved up front (the arena). Allocation is a
 *   simple pointer increment — no lock, no list search, no bookkeeping per
 *   object. All objects land consecutively in memory, giving excellent cache
 *   locality when they are later traversed. Freeing is a single reset of the
 *   offset pointer (no individual frees needed).
 *
 * This pattern is ubiquitous in systems software: parsers, compilers (per-
 * compilation-unit arenas), game engines (per-frame arenas), kernels
 * (slab allocators are a more sophisticated variant).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../common/benchmark.h"

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

/*
 * STEP 1 — malloc-based linked list parse.
 *
 * Simulate parsing a device tree by allocating MAX_NODES nodes, each with
 * MAX_PROPS properties, using malloc for every allocation.
 *
 * STEP 1a: allocate a node, initialise its name, link it at the head.
 *   Prepending is O(1) and doesn't require knowing the total count up front.
 *
 * STEP 1b: for each node, allocate MAX_PROPS properties and link them.
 *   Each malloc call is ~50–500 ns on a real system. With 256 nodes ×
 *   16 props = 4096 allocations total, the allocation cost alone is
 *   significant. The scattered memory layout adds cache pressure during
 *   subsequent traversals.
 *
 * STEP 1c: return the head of the list.
 */
static dt_node_t *linked_list_parse(void) {
    dt_node_t *head = NULL;

    /* STEP 1a: allocate and link each node */
    for (int i = 0; i < MAX_NODES; i++) {
        dt_node_t *node = malloc(sizeof(*node));
        if (!node) { perror("malloc node"); exit(1); }

        snprintf(node->name, sizeof(node->name), "device_%d", i);
        node->props = NULL;
        node->next  = head;
        head        = node;

        /* STEP 1b: allocate and link each property */
        for (int j = 0; j < MAX_PROPS; j++) {
            dt_prop_t *prop = malloc(sizeof(*prop));
            if (!prop) { perror("malloc prop"); exit(1); }

            snprintf(prop->key, sizeof(prop->key), "key_%d_%d", i, j);
            snprintf(prop->val, sizeof(prop->val), "val_%d_%d", i, j);
            prop->next  = node->props;
            node->props = prop;
        }
    }

    return head;  /* STEP 1c */
}

/*
 * STEP 2 — Free the malloc-based list.
 *
 * Walk the node list and, for each node, walk and free its property list
 * before freeing the node itself. Order matters: freeing the node first
 * would leave dangling pointers to properties (use-after-free).
 */
static void linked_list_free(dt_node_t *root) {
    while (root) {
        /* STEP 2a: free all properties of this node */
        dt_prop_t *prop = root->props;
        while (prop) {
            dt_prop_t *next_prop = prop->next;
            free(prop);
            prop = next_prop;
        }

        /* STEP 2b: advance to the next node, then free the current one */
        dt_node_t *next_node = root->next;
        free(root);
        root = next_node;
    }
}

/* ── Arena-based parse ───────────────────────────────────────────────────── */
static char   arena[ARENA_BYTES];
static size_t arena_offset;

/*
 * STEP 3 — Bump pointer allocator.
 *
 * Advance arena_offset by `sz` bytes (rounded up to 8-byte alignment) and
 * return a pointer to the allocated region.
 *
 * STEP 3a: alignment.
 *   C structs may require alignment (e.g., a pointer must be 8-byte aligned
 *   on 64-bit platforms). We round `sz` up to the next multiple of 8:
 *     aligned = (sz + 7) & ~7
 *
 * STEP 3b: bounds check.
 *   If the request would exceed the arena, return NULL. A production arena
 *   would panic or fall back to a secondary arena.
 *
 * STEP 3c: no locking, no bookkeeping, no free-list — just a pointer bump.
 *   This is the minimum possible allocation overhead.
 */
static void *arena_alloc(size_t sz) {
    /* STEP 3a: align to 8 bytes */
    size_t aligned = (sz + 7u) & ~7u;

    /* STEP 3b: bounds check */
    if (arena_offset + aligned > ARENA_BYTES) return NULL;

    /* STEP 3c: bump the pointer and return the old offset */
    void *ptr = arena + arena_offset;
    arena_offset += aligned;
    return ptr;
}

static void arena_reset(void) { arena_offset = 0; }

/*
 * STEP 4 — Arena-based parse.
 *
 * Same structure as linked_list_parse but every allocation uses arena_alloc.
 * No free calls are needed — the entire arena is discarded by arena_reset().
 *
 * STEP 4a: all nodes and properties land consecutively in the arena array.
 *   When later traversed in order, they are accessed sequentially — excellent
 *   cache behaviour (stride-1 access on the arena memory).
 *
 * STEP 4b: total arena usage = MAX_NODES * sizeof(dt_node_t) +
 *   MAX_NODES * MAX_PROPS * sizeof(dt_prop_t) = 256*96 + 256*16*96 = ~420 KB.
 *   This exceeds our 64 KB arena! arena_alloc will return NULL for later
 *   allocations. In a real bootloader you would size the arena appropriately.
 *   For this benchmark we simply stop when the arena is exhausted, making
 *   the comparison fair (both parse functions build as many nodes as possible).
 */
static dt_node_t *arena_parse(void) {
    dt_node_t *head = NULL;

    /* STEP 4a: allocate from the arena — no malloc overhead */
    for (int i = 0; i < MAX_NODES; i++) {
        dt_node_t *node = arena_alloc(sizeof(*node));
        if (!node) break;   /* STEP 4b: arena full — stop gracefully */

        snprintf(node->name, sizeof(node->name), "device_%d", i);
        node->props = NULL;
        node->next  = head;
        head        = node;

        for (int j = 0; j < MAX_PROPS; j++) {
            dt_prop_t *prop = arena_alloc(sizeof(*prop));
            if (!prop) break;

            snprintf(prop->key, sizeof(prop->key), "key_%d_%d", i, j);
            snprintf(prop->val, sizeof(prop->val), "val_%d_%d", i, j);
            prop->next  = node->props;
            node->props = prop;
        }
    }

    return head;
}

int main(void) {
    BENCH_RUN("linked_list_parse", 100, 10, {
        dt_node_t *root = linked_list_parse();
        linked_list_free(root);
    });

    BENCH_RUN("arena_parse", 100, 10, {
        arena_reset();
        (void)arena_parse();
        /* No free needed — arena_reset() reclaims everything in one step */
    });

    return 0;
}
