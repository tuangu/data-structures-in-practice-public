/*
 * SOLUTION: Exercise 9.1 — Binary Search Tree vs Sorted Array Lookup
 * Chapter 9: Binary Search Trees
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * Both structures support O(log n) search: the BST by following left/right
 * pointers, the sorted array by halving the search interval. But their cache
 * behaviour is very different:
 *
 *   BST (pointer-based):
 *     Each comparison requires dereferencing a pointer to an arbitrary heap
 *     address. For a 10 000-node BST the nodes span ~160 KB of heap. With a
 *     typical 32 KB L1 cache, almost every step down the tree is a cache miss.
 *     log2(10000) ≈ 13 hops × ~100 ns/miss = ~1.3 µs per lookup.
 *
 *   Sorted array + binary search:
 *     The array is contiguous. The first few comparisons jump to the middle,
 *     quarter-point, etc. — addresses that warm up predictably. By the final
 *     comparisons the candidate region fits entirely in a single cache line,
 *     making those accesses effectively free.
 *     The hardware prefetcher also helps with the early sequential-like pattern.
 *
 * Expected result: sorted array search is typically 2–5× faster on real hardware
 * despite performing the same number of comparisons. Under QEMU instruction
 * counts are nearly identical.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define N           10000
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

/*
 * STEP 1 — Iterative BST search.
 *
 * We use an *iterative* (not recursive) search to avoid function-call overhead
 * and stack frame allocation on each hop. The logic mirrors the recursive
 * version but uses a simple pointer walk.
 *
 * STEP 1a: start at the root, compare, descend left or right.
 *   - key < current->key → key is in the left subtree
 *   - key > current->key → key is in the right subtree
 *   - equal              → found; return 1
 *   - current == NULL    → key not in tree; return 0
 *
 * STEP 1b: the cache miss cost.
 *   Each iteration dereferences `current->left` or `current->right`. The
 *   next node's address is unknown until the current node is loaded. This
 *   creates a *load-use dependency chain* — the CPU cannot prefetch the next
 *   node because it doesn't know its address yet. Each hop potentially stalls.
 */
static int bst_search(bst_node_t *root, int key) {
    bst_node_t *cur = root;

    /* STEP 1c: walk down until we find the key or exhaust the tree */
    while (cur) {
        if (key == cur->key) return 1;          /* found */
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return 0;  /* not found */
}

/* ── Sorted array ────────────────────────────────────────────────────────── */
static int sorted[N];

/*
 * STEP 2 — Binary search on sorted array.
 *
 * Maintain a half-open interval [lo, hi). Each step computes the midpoint
 * and narrows the interval by half.
 *
 * STEP 2a: mid = lo + (hi - lo) / 2 avoids integer overflow that would
 *   occur with (lo + hi) / 2 if hi were near INT_MAX.
 *
 * STEP 2b: cache behaviour.
 *   The first comparison reads the midpoint of the array — a cold miss.
 *   The second comparison reads the midpoint of one half — a 50/50 chance
 *   of hitting the same cache line as the previous access. By the 4th–5th
 *   comparison the remaining region typically fits in L1. The total cache
 *   miss count per search is bounded by log2(cache_line_size / element_size)
 *   at steady state.
 */
static int arr_search(int key) {
    int lo = 0, hi = N;

    /* STEP 2c: half-open interval binary search */
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (sorted[mid] == key) return 1;      /* found */
        if (sorted[mid] < key)  lo = mid + 1;  /* key in upper half */
        else                    hi = mid;       /* key in lower half */
    }
    return 0;  /* not found */
}

/* ── Setup ───────────────────────────────────────────────────────────────── */
static int lookup_keys[NUM_LOOKUPS];

static void setup(void) {
    int keys[N];
    for (int i = 0; i < N; i++) keys[i] = i * 3 + 1;
    /* Knuth shuffle — random insertion order prevents degenerate BST */
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = keys[i]; keys[i] = keys[j]; keys[j] = t;
    }
    for (int i = 0; i < N; i++) bst_root = bst_insert(bst_root, keys[i]);

    for (int i = 0; i < N; i++) sorted[i] = i * 3 + 1;

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
