/*
 * Exercise 10.1: B-Tree Node Order vs Cache Performance
 * Chapter 10: B-Trees and Cache-Oblivious Structures
 *
 * Implement a simplified B-tree supporting integer keys.
 * Vary the node order (branching factor) and measure lookup performance.
 * Higher order → fewer pointer dereferences but larger nodes that may
 * span multiple cache lines.
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

#define MAX_ORDER    32     /* max branching factor to test */
#define INSERT_COUNT 100000
#define LOOKUP_COUNT 100000

/* ── Simplified B-tree (in-memory, for benchmarking only) ───────────────── */
typedef struct btree_node {
    int   *keys;
    struct btree_node **children;
    int    n_keys;
    int    is_leaf;
} btree_node_t;

typedef struct {
    btree_node_t *root;
    int           order;   /* max keys per node = order - 1 */
} btree_t;

static btree_node_t *bt_new_node(int order, int is_leaf) {
    btree_node_t *n = calloc(1, sizeof(*n));
    n->keys     = calloc((size_t)(order - 1), sizeof(int));
    n->children = calloc((size_t)order,       sizeof(btree_node_t *));
    n->is_leaf  = is_leaf;
    if (!n || !n->keys || !n->children) { perror("calloc"); exit(1); }
    return n;
}

/* TODO: implement btree_insert(bt, key) */
static void btree_insert(btree_t *bt, int key) {
    (void)bt; (void)key; /* TODO */
}

/* TODO: implement btree_search(bt, key); return 1 if found */
static int btree_search(const btree_t *bt, int key) {
    (void)bt; (void)key;
    return 0; /* TODO */
}

static void btree_init(btree_t *bt, int order) {
    bt->order = order;
    bt->root  = bt_new_node(order, 1);
}

/* ── Test harness ────────────────────────────────────────────────────────── */
static int lookup_keys[LOOKUP_COUNT];

int main(void) {
    for (int i = 0; i < LOOKUP_COUNT; i++)
        lookup_keys[i] = rand() % (INSERT_COUNT * 3);

    int orders[] = {4, 8, 16, 32};
    for (size_t o = 0; o < sizeof(orders)/sizeof(orders[0]); o++) {
        btree_t bt;
        btree_init(&bt, orders[o]);
        for (int i = 0; i < INSERT_COUNT; i++) btree_insert(&bt, i * 3);

        char label[64];
        snprintf(label, sizeof(label), "btree_lookup_order%d", orders[o]);

        volatile int sink = 0;
        BENCH_RUN(label, 20, 5, {
            for (int i = 0; i < LOOKUP_COUNT; i++)
                sink ^= btree_search(&bt, lookup_keys[i]);
        });
        (void)sink;
    }
    return 0;
}
