/*
 * SOLUTION: Exercise 16.1 — Bloom Filter
 * Chapter 16: Bloom Filters and Probabilistic Data Structures
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A Bloom filter answers "is this key possibly in the set?" with:
 *   - No false negatives: if a key was inserted, it is always found.
 *   - Controlled false positives: a key not inserted may still be reported
 *     as present (with probability p dependent on filter size and # hashes).
 *
 * How it works:
 *   Insert: compute k independent hash values for the key. Set bit[h_i % m]
 *     for each hash h_i (m = total bits in the filter).
 *   Query: compute the same k hashes. If ANY bit is 0, the key is absent.
 *     If ALL bits are 1, the key is "possibly present" (may be a false positive).
 *
 * False positive rate formula:
 *   p ≈ (1 - e^(-k*n/m))^k
 *   where n = number of inserted elements, m = filter bits, k = hash functions.
 *
 * With FILTER_BITS=65536, NUM_HASH=3, INSERT_COUNT=10000:
 *   p ≈ (1 - e^(-3*10000/65536))^3 ≈ (1 - e^(-0.458))^3 ≈ (0.367)^3 ≈ 4.9%
 *
 * The hash function must be independent for each seed value. We use a
 * seeded FNV-1a variant: mix the seed into the offset basis so that
 * different seeds produce uncorrelated output for the same key.
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

/*
 * STEP 1 — Seeded FNV-1a hash function.
 *
 * We need k independent hash functions. A common technique is to use a single
 * hash function with k different seeds. For FNV-1a, we mix the seed into the
 * offset basis so the starting state differs per seed, producing decorrelated
 * output.
 *
 * STEP 1a: seed mixing.
 *   Start from the FNV offset basis, then XOR with the seed and multiply by
 *   the prime once. This "primes" the state differently for each seed value.
 *
 * STEP 1b: process the key one byte at a time using the standard FNV-1a loop.
 *   h ^= byte; h *= prime;
 *   Each byte is folded into all subsequent state through multiplication.
 *
 * The result is the bit index to set or check. The caller will take this
 * modulo FILTER_BITS to get the actual position.
 */
static uint32_t bloom_hash(uint32_t key, int seed) {
    /* STEP 1a: seeded offset basis — each seed gives a different start state */
    uint32_t h = 2166136261u ^ (uint32_t)seed;
    h *= 16777619u;

    /* STEP 1b: mix the 4 bytes of `key` using FNV-1a */
    h ^= (key & 0xFFu);         h *= 16777619u;
    h ^= ((key >> 8) & 0xFFu);  h *= 16777619u;
    h ^= ((key >> 16) & 0xFFu); h *= 16777619u;
    h ^= ((key >> 24) & 0xFFu); h *= 16777619u;

    return h;
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

    /*
     * STEP 2 — False positive measurement.
     *
     * Query keys in the range [INSERT_COUNT, 2*INSERT_COUNT). None of these
     * were inserted, so every "found" result is a false positive.
     *
     * Expected rate with the default parameters: ~4–6%.
     * Try adjusting FILTER_BITS (larger = lower rate) or NUM_HASH and observe
     * how the rate changes vs the theoretical formula.
     */
    int fp = 0;
    for (uint32_t i = INSERT_COUNT; i < INSERT_COUNT * 2; i++)
        if (bloom_query(i)) fp++;

    printf("FILTER_BITS=%-6d  NUM_HASH=%d  inserted=%d  "
           "false_positives=%d  rate=%.2f%%\n",
           FILTER_BITS, NUM_HASH, INSERT_COUNT,
           fp, 100.0 * fp / INSERT_COUNT);

    /*
     * STEP 3 — Tuning guide (printed for the learner).
     *
     * The optimal number of hash functions for a given (m, n) is:
     *   k_opt = (m/n) * ln(2) ≈ 0.693 * (m/n)
     *
     * With m=65536 and n=10000: k_opt ≈ 0.693 * 6.55 ≈ 4.5 → try k=4 or k=5.
     */
    printf("Optimal k for this (m=%d, n=%d): k_opt = %.1f\n",
           FILTER_BITS, INSERT_COUNT,
           0.693 * ((double)FILTER_BITS / INSERT_COUNT));

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
