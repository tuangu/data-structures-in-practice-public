/*
 * SOLUTION: Exercise 10.1 — B-Tree Node Order vs Cache Performance
 * Chapter 10: B-Trees and Cache-Oblivious Structures
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A B-tree of order t stores between t-1 and 2t-1 keys per node. Higher order
 * means fewer levels (fewer pointer dereferences = fewer cache misses) but
 * larger nodes (more keys to scan linearly within a node = more cache lines
 * touched per node visit). There is an optimal order where node size matches
 * the cache line size or a small multiple of it.
 *
 * Implementation: "split-on-the-way-down" (proactive splitting).
 *   Before descending into a full child, we split it. This ensures we never
 *   need to split a node on the way back up, making the algorithm iterative-
 *   friendly and avoiding a second tree traversal.
 *
 * This solution implements a simplified B-tree (not a B+ tree — data is stored
 * in internal nodes as well as leaves). For benchmarking purposes the structure
 * is correct; production systems typically use B+ trees for better leaf-scan
 * performance.
 *
 * Walkthrough structure:
 *   bt_split_child     — split a full child into two half-full children
 *   btree_insert_nonfull — insert into a non-full node, splitting on descent
 *   btree_insert       — handle the root-full case, then delegate
 *   btree_search       — iterative search: linear scan within node + descent
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define MAX_ORDER    32
#define INSERT_COUNT 100000
#define LOOKUP_COUNT 100000

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

/*
 * STEP 1 — Split a full child.
 *
 * When a child node is full (n_keys == order - 1), we split it into two
 * nodes of (order/2 - 1) keys each and promote the median key up to the
 * parent. The parent must NOT be full (guaranteed by split-on-the-way-down).
 *
 * Parameters:
 *   parent  — the non-full node that owns the full child
 *   i       — index of the full child in parent->children[]
 *   order   — maximum branching factor
 *
 * STEP 1a: identify the full child and its median.
 *   The median key is at index (t-1) where t = order/2.
 *   Keys [0..t-2]   stay in the left (original) child.
 *   Keys [t..2t-2]  move to the new right child.
 *   Key  [t-1]      is promoted to the parent.
 *
 * STEP 1b: create the right sibling with the upper half of keys.
 *
 * STEP 1c: insert the median into the parent at position i,
 *   then insert the new right child at position i+1.
 */
static void bt_split_child(btree_node_t *parent, int i, int order) {
    int t = order / 2;                            /* minimum degree */
    btree_node_t *full  = parent->children[i];
    btree_node_t *right = bt_new_node(order, full->is_leaf);

    /* STEP 1b: copy upper keys to the new right node */
    right->n_keys = t - 1;
    for (int j = 0; j < t - 1; j++)
        right->keys[j] = full->keys[j + t];

    /* STEP 1b cont: copy upper children if not a leaf */
    if (!full->is_leaf)
        for (int j = 0; j < t; j++)
            right->children[j] = full->children[j + t];

    full->n_keys = t - 1;   /* left node now holds only the lower keys */

    /* STEP 1c: make room in parent for the promoted median key and new child */
    for (int j = parent->n_keys; j > i; j--)
        parent->children[j + 1] = parent->children[j];
    parent->children[i + 1] = right;

    for (int j = parent->n_keys - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = full->keys[t - 1];   /* promote the median */
    parent->n_keys++;
}

/*
 * STEP 2 — Insert into a non-full node (recursive helper).
 *
 * We know this node is not full. Either:
 *   a) It is a leaf: find the correct position and insert the key.
 *   b) It is an internal node: find the correct child, split it if full
 *      (proactively — so we never go back up), then recurse.
 *
 * STEP 2a: find the rightmost key position where key >= keys[i].
 *   We start from n_keys-1 and scan left until we find the insertion point.
 *
 * STEP 2b: leaf insertion.
 *   Shift keys right to open a gap, then write the new key.
 *
 * STEP 2c: internal node — find child, split if full, recurse.
 */
static void btree_insert_nonfull(btree_node_t *node, int key, int order) {
    int i = node->n_keys - 1;

    if (node->is_leaf) {
        /* STEP 2b: shift keys right to make room */
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->n_keys++;
    } else {
        /* STEP 2a: find which child to descend into */
        while (i >= 0 && key < node->keys[i]) i--;
        i++;  /* i is now the index of the child to follow */

        /* STEP 2c: split the child proactively if it is full */
        if (node->children[i]->n_keys == order - 1) {
            bt_split_child(node, i, order);
            /* after split, the median moved up; choose left or right child */
            if (key > node->keys[i]) i++;
        }
        btree_insert_nonfull(node->children[i], key, order);
    }
}

/*
 * STEP 3 — Public insert.
 *
 * If the root is full, we cannot insert into it directly. Instead:
 *   - Allocate a new root.
 *   - Make the old root the new root's only child.
 *   - Split that child (which promotes one key to the new root).
 *   - Insert the key into the appropriate half.
 *
 * If the root is not full, delegate to btree_insert_nonfull directly.
 */
static void btree_insert(btree_t *bt, int key) {
    btree_node_t *r = bt->root;

    if (r->n_keys == bt->order - 1) {
        /* STEP 3a: root is full — grow the tree upward */
        btree_node_t *new_root = bt_new_node(bt->order, 0 /* not leaf */);
        new_root->children[0] = r;
        bt->root = new_root;
        bt_split_child(new_root, 0, bt->order);
        btree_insert_nonfull(new_root, key, bt->order);
    } else {
        btree_insert_nonfull(r, key, bt->order);
    }
}

/*
 * STEP 4 — Search.
 *
 * Iterative search: at each node, do a linear scan of keys to find either
 * the key itself or the child to descend into.
 *
 * STEP 4a: linear scan vs binary search within a node.
 *   For small orders (4, 8) a linear scan is fine and avoids the branch
 *   overhead of binary search within a tiny array. For larger orders (16, 32)
 *   a binary search within the node would save comparisons. We use linear scan
 *   here for clarity; the benchmark will reveal whether order matters.
 *
 * STEP 4b: cache observation.
 *   Each node visit touches node->keys[] (n_keys ints) and then
 *   node->children[i] (one pointer). For order=4 a node is tiny (~32 bytes);
 *   for order=32 a node's key array is 124 bytes (> 2 cache lines). Higher
 *   orders reduce tree height but increase per-node scan cost.
 */
static int btree_search(const btree_t *bt, int key) {
    btree_node_t *cur = bt->root;

    /* STEP 4c: descend until we find the key or reach a null child */
    while (cur) {
        int i = 0;
        /* STEP 4a: linear scan within the node */
        while (i < cur->n_keys && key > cur->keys[i]) i++;

        if (i < cur->n_keys && key == cur->keys[i])
            return 1;  /* found */

        if (cur->is_leaf)
            return 0;  /* not in tree */

        cur = cur->children[i];   /* STEP 4b: descend to the correct child */
    }
    return 0;
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
