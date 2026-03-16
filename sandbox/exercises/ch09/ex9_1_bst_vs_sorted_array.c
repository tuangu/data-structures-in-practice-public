/*
 * Exercise 9.1: Binary Search Tree vs Sorted Array Lookup
 * Chapter 9: Binary Search Trees
 *
 * 10 000 element lookup measured for:
 *   A) Iterative BST (pointer chasing — poor cache behaviour)
 *   B) Sorted array + binary search (sequential prefetch-friendly)
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../common/benchmark.h"

#define N         10000
#define NUM_LOOKUPS 10000

/* ── BST ─────────────────────────────────────────────────────────────────── */
typedef struct bst_node {
    int key;
    struct bst_node *left, *right;
} bst_node_t;

static bst_node_t *bst_root = NULL;

static bst_node_t *bst_insert(bst_node_t *root, int key) {
    if (!root) {
        bst_node_t *n = malloc(sizeof(*n));
        if (!n) { perror("malloc"); exit(1); }
        n->key = key; n->left = n->right = NULL;
        return n;
    }
    if (key < root->key) root->left  = bst_insert(root->left,  key);
    else if (key > root->key) root->right = bst_insert(root->right, key);
    return root;
}

/* TODO: implement iterative BST search; return 1 if found, 0 otherwise */
static int bst_search(bst_node_t *root, int key) {
    (void)root; (void)key;
    return 0; /* TODO */
}

/* ── Sorted array ────────────────────────────────────────────────────────── */
static int sorted[N];

/* TODO: implement binary search on sorted[]; return 1 if found */
static int arr_search(int key) {
    (void)key;
    return 0; /* TODO */
}

/* ── Setup ───────────────────────────────────────────────────────────────── */
static int lookup_keys[NUM_LOOKUPS];

static void setup(void) {
    /* Insert keys in random order to avoid degenerate BST */
    int keys[N];
    for (int i = 0; i < N; i++) keys[i] = i * 3 + 1;
    /* Knuth shuffle */
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = keys[i]; keys[i] = keys[j]; keys[j] = t;
    }
    for (int i = 0; i < N; i++) bst_root = bst_insert(bst_root, keys[i]);

    for (int i = 0; i < N; i++) sorted[i] = i * 3 + 1;
    /* sorted[] is already sorted by construction */

    for (int i = 0; i < NUM_LOOKUPS; i++)
        lookup_keys[i] = sorted[rand() % N];
}

int main(void) {
    setup();

    volatile int sink = 0;
    BENCH_RUN("bst_search",        500, 50, {
        for (int i = 0; i < NUM_LOOKUPS; i++) sink ^= bst_search(bst_root, lookup_keys[i]);
    });
    BENCH_RUN("sorted_arr_search", 500, 50, {
        for (int i = 0; i < NUM_LOOKUPS; i++) sink ^= arr_search(lookup_keys[i]);
    });
    (void)sink;
    return 0;
}
