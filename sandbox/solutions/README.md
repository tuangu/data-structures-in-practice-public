# Exercise Solutions

This directory contains reference solutions for every exercise in `../exercises/`.
Each solution file is a **drop-in replacement** for the corresponding exercise file
and can be built with the same `Makefile` by changing the source path.

---

## How to Use These Solutions

### 1. Try the exercise first

Open the exercise file, read the `TODO` comments, and write your implementation.
Build and run it:

```bash
cd sandbox
make ARCH=riscv64 all        # or ARCH=x86_64
./run.sh build/riscv64/ch01/ex1_1_hash_vs_bsearch
```

### 2. Compare against the solution

Each solution file contains **step-by-step walkthrough comments** above every
function that explain:
- *What* the implementation does
- *Why* each line is written the way it is
- *What hardware behaviour* the exercise is trying to demonstrate

Open the solution in your editor alongside the exercise:

```
exercises/ch01/ex1_1_hash_vs_bsearch.c   ← your work
solutions/ch01/ex1_1_hash_vs_bsearch.c   ← reference
```

### 3. Build a solution directly

To compile a solution without modifying the Makefile, supply the path explicitly:

```bash
# Example: build the ch01 ex1_1 solution for RISC-V
cd sandbox
riscv64-linux-gnu-gcc -march=rv64gc -mabi=lp64d -O2 -Wall -Wextra -g \
    -Iexercises/common \
    solutions/ch01/ex1_1_hash_vs_bsearch.c \
    exercises/common/benchmark.c \
    -o /tmp/ex1_1_sol -lpthread -lm

qemu-riscv64-static -L /usr/riscv64-linux-gnu /tmp/ex1_1_sol

# x86-64 version
gcc -O2 -Wall -Wextra -g \
    -Iexercises/common \
    solutions/ch01/ex1_1_hash_vs_bsearch.c \
    exercises/common/benchmark.c \
    -o /tmp/ex1_1_sol -lpthread -lm && /tmp/ex1_1_sol
```

Inside Docker:

```bash
docker compose run --rm sandbox bash -c "
  gcc -O2 -Wall -Wextra -g -Iexercises/common \
      solutions/ch01/ex1_1_hash_vs_bsearch.c \
      exercises/common/benchmark.c \
      -o /tmp/ex1_1_sol -lpthread -lm && /tmp/ex1_1_sol"
```

---

## Solution Index

| Chapter | Exercise file | Key concept |
|---|---|---|
| 1 | `ch01/ex1_1_hash_vs_bsearch.c` | Binary search vs hash lookup |
| 1 | `ch01/ex1_2_cache_miss.c` | Sequential vs random access |
| 2 | `ch02/ex2_1_cache_line_detect.c` | Stride access to detect cache line size |
| 2 | `ch02/ex2_2_false_sharing.c` | False sharing with thread padding |
| 3 | `ch03/ex3_1_microbenchmark.c` | Benchmark framework + linked-list workload |
| 4 | `ch04/ex4_1_matrix_traversal.c` | Row-major vs column-major traversal |
| 4 | `ch04/ex4_2_soa_vs_aos.c` | SoA vs AoS particle update |
| 5 | `ch05/ex5_1_list_vs_array.c` | Array stack vs linked-list stack |
| 6 | `ch06/ex6_1_ring_buffer.c` | Ring buffer with bitmask modulo |
| 7 | `ch07/ex7_1_hash_quality.c` | Byte-sum / FNV-1a / Murmur3 hash quality |
| 8 | `ch08/ex8_1_dynamic_array.c` | Dynamic array growth strategies |
| 9 | `ch09/ex9_1_bst_vs_sorted_array.c` | Iterative BST search vs binary search |
| 10 | `ch10/ex10_1_btree_cache.c` | B-tree insert + search (split-on-the-way-down) |
| 12 | `ch12/ex12_1_heap_variants.c` | d-ary heap insert + extract-min |
| 13 | `ch13/ex13_1_lockfree_queue.c` | SPSC lock-free queue vs mutex queue |
| 16 | `ch16/ex16_1_bloom_filter.c` | Bloom filter with FNV-1a seeded hash |
| 17 | `ch17/ex17_1_bootloader_sim.c` | malloc parse vs arena-bump-allocator parse |

---

## Reading the Walkthrough Comments

Solution files use a consistent annotation style:

```
/* STEP N: <one-sentence summary>
 *
 * <explanation of why this is the right approach>
 * <connection to the hardware concept from the chapter>
 */
```

Look for these blocks immediately before each non-trivial line or group of lines.
