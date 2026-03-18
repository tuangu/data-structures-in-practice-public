# Sandbox — Exercise Environment

This directory contains all hands-on exercises from *Data Structures in Practice*.
Each exercise is a self-contained C program that compiles and runs for either
**RISC-V 64-bit** (default) or **x86-64**, letting you observe hardware-level
behaviour described in the book.

---

## Directory Layout

```
sandbox/
├── Dockerfile          # Container image with cross-compiler + QEMU
├── docker-compose.yml  # Convenience wrapper for Docker Compose
├── Makefile            # Build system (supports ARCH=riscv64 | x86_64)
├── run.sh              # Helper script to run RISC-V binaries via QEMU
└── exercises/
    ├── common/
    │   ├── benchmark.h   # Shared timing & reporting helpers
    │   └── benchmark.c
    └── chNN/
        └── exNN_<name>.c # One file per exercise
```

---

## Quick Start — Docker (Recommended)

Docker provides a pre-built environment with the RISC-V cross-compiler and QEMU
already installed, so nothing extra is needed on your host.

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) 20.10+
- [Docker Compose](https://docs.docker.com/compose/install/) v2+

### Build and run all exercises (RISC-V)

```bash
cd sandbox
docker compose run --rm sandbox make ARCH=riscv64 run
```

### Build only

```bash
docker compose run --rm sandbox make ARCH=riscv64 all
```

### Open an interactive shell inside the container

```bash
docker compose run --rm sandbox bash
# Inside the container:
make ARCH=riscv64 all
make ARCH=riscv64 run
```

---

## Quick Start — Native Host (Linux)

If you prefer to run without Docker, install the required tools first.

### Prerequisites — RISC-V cross-compilation (Debian/Ubuntu)

```bash
sudo apt-get install \
    gcc-riscv64-linux-gnu \
    binutils-riscv64-linux-gnu \
    qemu-user-static \
    libc6-dev-riscv64-cross \
    make
```

### Prerequisites — x86-64 native

```bash
sudo apt-get install gcc make
```

### Build all exercises

```bash
cd sandbox

# RISC-V (default)
make ARCH=riscv64 all

# x86-64
make ARCH=x86_64 all
```

### Run all exercises

```bash
# RISC-V — via QEMU user-space emulation
make ARCH=riscv64 run

# x86-64 — runs natively
make ARCH=x86_64 run
```

---

## Running Individual Exercises

### With the Makefile runner

```bash
# Build first, then run one binary directly
make ARCH=riscv64 all
./run.sh build/riscv64/ch01/ex1_1_hash_vs_bsearch
```

### With run.sh (RISC-V only)

```bash
# All built binaries
./run.sh

# A single binary
./run.sh build/riscv64/ch07/ex7_1_hash_quality
```

### x86-64 binaries run directly

```bash
./build/x86_64/ch04/ex4_2_soa_vs_aos
```

---

## Architecture Options

| `ARCH` value | Compiler | Target | Runner |
|---|---|---|---|
| `riscv64` (default) | `riscv64-linux-gnu-gcc` | RISC-V 64-bit (`rv64gc`) | `qemu-riscv64-static` |
| `x86_64` | `gcc` | Native x86-64 | (none — runs directly) |

---

## Exercise Map

| Chapter | File | Topic |
|---|---|---|
| 1 | `ex1_1_hash_vs_bsearch.c` | Hash table vs binary search performance |
| 1 | `ex1_2_cache_miss.c` | Cache miss cost measurement |
| 2 | `ex2_1_cache_line_detect.c` | Cache line size detection |
| 2 | `ex2_2_false_sharing.c` | False sharing between threads |
| 3 | `ex3_1_microbenchmark.c` | Microbenchmark methodology |
| 4 | `ex4_1_matrix_traversal.c` | Row-major vs column-major access |
| 4 | `ex4_2_soa_vs_aos.c` | Struct-of-arrays vs array-of-structs |
| 5 | `ex5_1_list_vs_array.c` | Linked list vs array traversal |
| 6 | `ex6_1_ring_buffer.c` | Ring buffer implementation |
| 7 | `ex7_1_hash_quality.c` | Hash function quality analysis |
| 8 | `ex8_1_dynamic_array.c` | Dynamic array growth strategies |
| 9 | `ex9_1_bst_vs_sorted_array.c` | BST vs sorted array search |
| 10 | `ex10_1_btree_cache.c` | B-tree cache efficiency |
| 12 | `ex12_1_heap_variants.c` | Binary heap variants |
| 13 | `ex13_1_lockfree_queue.c` | Lock-free queue with atomics |
| 16 | `ex16_1_bloom_filter.c` | Bloom filter false-positive rate |
| 17 | `ex17_1_bootloader_sim.c` | Bootloader data structure simulation |

---

## Common Make Targets

| Target | Description |
|---|---|
| `make all` | Build all exercises (default: `ARCH=riscv64`) |
| `make run` | Build then run all exercises |
| `make clean` | Remove the `build/` directory |

---

## Troubleshooting

**`riscv64-linux-gnu-gcc: command not found`**
Install the cross-compiler package shown above, or use Docker.

**`qemu-riscv64-static: command not found`**
Install `qemu-user-static`, or use Docker.

**Binary exits immediately with a signal**
Run with `ARCH=x86_64` to rule out emulation issues, then compare results.

**`make: Nothing to be done for 'all'`**
Sources are unchanged. Run `make clean` then `make all` to force a rebuild.
