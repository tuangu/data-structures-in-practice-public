/*
 * SOLUTION: Exercise 4.2 — Structure-of-Arrays vs Array-of-Structures
 * Chapter 4: Arrays and Cache Locality
 *
 * ── Walkthrough overview ────────────────────────────────────────────────────
 *
 * A particle system updates positions each frame: x += vx*dt.
 * Two data layouts are compared:
 *
 *   AoS (Array-of-Structures):
 *     struct { float x, y, z, mass; } particles[N]
 *     Memory: x0 y0 z0 m0 | x1 y1 z1 m1 | x2 y2 z2 m2 | …
 *     A 64-byte cache line holds 4 particles (16 floats). But the update loop
 *     only reads x, y, z — `mass` is loaded into the cache line but never
 *     used. 25% of cache bandwidth is wasted on unused data.
 *
 *   SoA (Structure-of-Arrays):
 *     float xs[N], ys[N], zs[N]   (mass array omitted from update)
 *     Memory: x0 x1 x2 … | y0 y1 y2 … | z0 z1 z2 …
 *     A 64-byte cache line holds 16 x-values. Every byte loaded is used.
 *     Additionally, xs/ys/zs are independent arrays — SIMD vectorisation
 *     can process 4 or 8 floats per instruction.
 *
 * Expected result: SoA is ~1.3–2× faster on real hardware.
 * Under QEMU (instruction counting): similar counts since the loop body
 * is the same number of loads/stores in both cases.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define N 100000

/* ── Array-of-Structures ─────────────────────────────────────────────────── */
typedef struct {
    float x, y, z;
    float mass;    /* not used in position update — wastes cache bandwidth */
} particle_aos_t;

static particle_aos_t aos[N];
static float vx[N], vy[N], vz[N];
static const float dt = 0.001f;

/* ── Structure-of-Arrays ─────────────────────────────────────────────────── */
static float soa_x[N], soa_y[N], soa_z[N];

static void setup(void) {
    for (int i = 0; i < N; i++) {
        aos[i].x = aos[i].y = aos[i].z = (float)i;
        aos[i].mass = 1.0f;
        vx[i] = vy[i] = vz[i] = 0.1f;
        soa_x[i] = soa_y[i] = soa_z[i] = (float)i;
    }
}

/*
 * STEP 1 — AoS position update.
 *
 * For each particle we update three fields of the same struct. The struct is
 * 16 bytes wide (x, y, z, mass — 4 floats), so two particles fit per cache
 * line. The `mass` field is loaded with each cache line but never read here,
 * wasting 1/4 of all cache bandwidth.
 *
 * STEP 1a: access order within the struct (x then y then z) matches the
 * memory layout, so there is no extra penalty within a single particle —
 * the penalty is the unused `mass` field loaded alongside.
 */
static void update_aos(void) {
    /* STEP 1b: straightforward field access — three stores per particle */
    for (int i = 0; i < N; i++) {
        aos[i].x += vx[i] * dt;
        aos[i].y += vy[i] * dt;
        aos[i].z += vz[i] * dt;
    }
}

/*
 * STEP 2 — SoA position update.
 *
 * Each array (soa_x, soa_y, soa_z) is independent. When processing soa_x,
 * a 64-byte cache line brings in 16 consecutive x-values — all are used.
 * No wasted bandwidth. The compiler can also auto-vectorise three independent
 * loops more easily (no struct field aliasing to reason about).
 *
 * STEP 2a: the three loops look identical to update_aos but address
 * completely separate memory regions, maximising cache utilisation.
 */
static void update_soa(void) {
    /* STEP 2b: x, y, z arrays are separate — full cache line utilisation */
    for (int i = 0; i < N; i++)
        soa_x[i] += vx[i] * dt;

    for (int i = 0; i < N; i++)
        soa_y[i] += vy[i] * dt;

    for (int i = 0; i < N; i++)
        soa_z[i] += vz[i] * dt;
}

int main(void) {
    setup();
    BENCH_RUN("position_update_aos", 100, 10, { update_aos(); });
    BENCH_RUN("position_update_soa", 100, 10, { update_soa(); });
    return 0;
}
