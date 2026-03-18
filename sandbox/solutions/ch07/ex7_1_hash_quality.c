/*
 * SOLUTION: Exercise 7.1 — Hash Function Quality Comparison
 * Chapter 7: Hash Tables
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A good hash function must distribute keys uniformly across buckets to
 * minimise collisions. We compare three functions by computing the variance
 * of bucket occupancy counts: lower variance = more uniform = fewer collisions.
 *
 * Expected results (approximate, with TABLE_SIZE=1024, NUM_KEYS=10000):
 *   sum_of_bytes:  high variance — byte sums cluster around a mean, few bits
 *                  in the sum carry information about key structure.
 *   fnv1a_32:      low variance — mixes each byte through multiplication and
 *                  XOR, avalanche effect propagates differences.
 *   murmur3_mix:   very low variance — finaliser mix designed for maximum
 *                  avalanche with minimal computation.
 *
 * Performance: sum_of_bytes is cheapest (loop + add); FNV-1a adds a multiply
 * per byte; Murmur3 mix uses shifts and multiplies but processes all bits at
 * once as a block hash rather than per-byte.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "../../common/benchmark.h"

#define TABLE_SIZE  1024
#define NUM_KEYS    10000
#define KEY_LEN     16

static char keys[NUM_KEYS][KEY_LEN];

static void setup(void) {
    uint32_t state = 0xdeadbeef;
    for (int i = 0; i < NUM_KEYS; i++) {
        for (int j = 0; j < KEY_LEN - 1; j++) {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            keys[i][j] = 'a' + (char)(state % 26);
        }
        keys[i][KEY_LEN - 1] = '\0';
    }
}

/* ── Hash functions ──────────────────────────────────────────────────────── */

/*
 * STEP 1 — Naive byte-sum hash.
 *
 * Iterate over each byte of the key and add it to the accumulator.
 *
 * Why this is poor:
 *   - Anagrams ("abc" and "bca") hash to the same bucket.
 *   - The sum of printable ASCII bytes is bounded: each byte in ['a'..'z']
 *     contributes 97–122. A 15-char key sums to at most 15 × 122 = 1830.
 *     With TABLE_SIZE=1024 the upper ~44% of buckets can never be reached,
 *     while lower buckets are over-used. Very poor uniformity.
 */
static uint32_t hash_sum(const char *key) {
    uint32_t h = 0;

    /* STEP 1a: accumulate bytes — order-independent, anagram-blind */
    while (*key)
        h += (unsigned char)*key++;

    return h;
}

/*
 * STEP 2 — FNV-1a 32-bit hash.
 *
 * FNV-1a (Fowler–Noll–Vo variant 1a) processes each byte with:
 *   hash = (hash XOR byte) * FNV_PRIME
 *
 * The XOR-then-multiply order (1a) provides better avalanche than the
 * original multiply-then-XOR order (1). Each byte fully participates in
 * all subsequent multiplications.
 *
 * Constants:
 *   FNV_OFFSET_BASIS = 2166136261 (32-bit)
 *   FNV_PRIME        = 16777619   (32-bit)
 * These are chosen so that multiplication by the prime has maximum
 * bit-mixing properties modulo 2^32.
 */
static uint32_t hash_fnv1a(const char *key) {
    /* STEP 2a: initialise with the FNV offset basis */
    uint32_t h = 2166136261u;

    /* STEP 2b: XOR each byte into the hash, then multiply by the prime */
    while (*key) {
        h ^= (unsigned char)*key++;   /* fold byte into hash */
        h *= 16777619u;               /* mix — every prior byte influences h */
    }

    return h;
}

/*
 * STEP 3 — Murmur3 32-bit finaliser mix.
 *
 * Rather than hashing byte-by-byte, we accumulate all bytes into a 32-bit
 * value and apply the Murmur3 finaliser to mix the bits. The finaliser is:
 *   h ^= h >> 16
 *   h *= 0x85ebca6b
 *   h ^= h >> 13
 *   h *= 0xc2b2ae35
 *   h ^= h >> 16
 *
 * This is the "finalisation mix" from the Murmur3 algorithm — three XOR-shift
 * steps separated by multiplications with carefully chosen constants. Each
 * step ensures every input bit affects every output bit (avalanche property).
 *
 * We seed the initial accumulation with a position-dependent multiplier
 * (i+1) so that anagrams produce different pre-mix values.
 */
static uint32_t hash_murmur3(const char *key) {
    uint32_t h = 0;
    int i = 0;

    /* STEP 3a: accumulate bytes with a position-dependent contribution
     * to make the function order-sensitive (unlike hash_sum) */
    while (*key) {
        h += (unsigned char)*key++ * (uint32_t)(i + 1);
        i++;
    }

    /* STEP 3b: Murmur3 finaliser — bit-mixing cascade */
    h ^= h >> 16;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    h *= 0xc2b2ae35u;
    h ^= h >> 16;

    return h;
}

/* ── Measure variance ────────────────────────────────────────────────────── */
typedef uint32_t (*hash_fn_t)(const char *);

static double bucket_variance(hash_fn_t fn) {
    int counts[TABLE_SIZE] = {0};
    for (int i = 0; i < NUM_KEYS; i++)
        counts[fn(keys[i]) % TABLE_SIZE]++;
    double mean = (double)NUM_KEYS / TABLE_SIZE;
    double var = 0.0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        double d = counts[i] - mean;
        var += d * d;
    }
    return var / TABLE_SIZE;
}

int main(void) {
    setup();

    printf("%-20s  variance\n", "hash_function");
    printf("%-20s  %.2f\n", "sum_of_bytes",  bucket_variance(hash_sum));
    printf("%-20s  %.2f\n", "fnv1a_32",      bucket_variance(hash_fnv1a));
    printf("%-20s  %.2f\n", "murmur3_mix",   bucket_variance(hash_murmur3));

    volatile uint32_t sink = 0;
    BENCH_RUN("hash_sum_10k",    200, 20, {
        for (int i = 0; i < NUM_KEYS; i++) sink ^= hash_sum(keys[i]);
    });
    BENCH_RUN("hash_fnv1a_10k",  200, 20, {
        for (int i = 0; i < NUM_KEYS; i++) sink ^= hash_fnv1a(keys[i]);
    });
    BENCH_RUN("hash_murmur_10k", 200, 20, {
        for (int i = 0; i < NUM_KEYS; i++) sink ^= hash_murmur3(keys[i]);
    });
    (void)sink;
    return 0;
}
