/*
 * Exercise 5.1: Linked List Stack vs Array Stack
 * Chapter 5: Linked Lists
 *
 * Implement a stack two ways:
 *   A) Array with a top index (cache-friendly, no malloc per op)
 *   B) Singly-linked list (malloc/free per push/pop, poor locality)
 *
 * Measure 10 000 push followed by 10 000 pop operations for each.
 *
 * Build:
 *   make ARCH=riscv64
 *   make ARCH=x86_64
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../common/benchmark.h"

#define N 10000

/* ── Array stack ─────────────────────────────────────────────────────────── */
static int arr_stack[N];
static int arr_top = -1;

static void arr_push(int val) {
    /* TODO */
    (void)val;
}

static int arr_pop(void) {
    return 0; /* TODO */
}

/* ── Linked-list stack ───────────────────────────────────────────────────── */
typedef struct node { int val; struct node *next; } node_t;
static node_t *list_head = NULL;

static void list_push(int val) {
    /* TODO: malloc a node, link it at head */
    (void)val;
}

static int list_pop(void) {
    /* TODO: unlink head, free, return val */
    return 0;
}

/* ── Workloads ───────────────────────────────────────────────────────────── */
static void workload_arr(void) {
    for (int i = 0; i < N; i++) arr_push(i);
    for (int i = 0; i < N; i++) (void)arr_pop();
}

static void workload_list(void) {
    for (int i = 0; i < N; i++) list_push(i);
    for (int i = 0; i < N; i++) (void)list_pop();
}

int main(void) {
    BENCH_RUN("array_stack_push_pop",  200, 20, { workload_arr(); });
    BENCH_RUN("list_stack_push_pop",   200, 20, { workload_list(); });
    return 0;
}
