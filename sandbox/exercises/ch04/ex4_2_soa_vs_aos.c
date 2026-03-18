/*
 * Exercise 4.2: Structure-of-Arrays vs Array-of-Structures
 * Chapter 4: Arrays and Cache Locality
 *
 * Simulate a particle system. Compare:
 *   AoS: struct { float x, y, z, mass; } particles[N]
 *   SoA: float xs[N], ys[N], zs[N], masses[N]
 *
 * The position-update inner loop (x += vx * dt) only touches x, y, z.
 * With AoS every cache line also pulls in mass (wasted bandwidth).
 * With SoA only the xs[], ys[], zs[] arrays are touched.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../common/benchmark.h"

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
/* (mass array omitted — not read by position update) */

static void setup(void) {
    for (int i = 0; i < N; i++) {
        aos[i].x = aos[i].y = aos[i].z = (float)i;
        aos[i].mass = 1.0f;
        vx[i] = vy[i] = vz[i] = 0.1f;
        soa_x[i] = soa_y[i] = soa_z[i] = (float)i;
    }
}

/* TODO: update positions for all particles using AoS layout */
static void update_aos(void) {
    /* TODO: for each i: aos[i].x += vx[i]*dt; ... */
}

/* TODO: update positions for all particles using SoA layout */
static void update_soa(void) {
    /* TODO: for each i: soa_x[i] += vx[i]*dt; ... */
}

int main(void) {
    setup();
    BENCH_RUN("position_update_aos", 100, 10, { update_aos(); });
    BENCH_RUN("position_update_soa", 100, 10, { update_soa(); });
    return 0;
}
