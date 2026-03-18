#!/bin/sh
# run.sh — run RISC-V exercise binaries through qemu-riscv64-static
#
# Usage:
#   ./run.sh                                    # run all built binaries
#   ./run.sh build/riscv64/ch01/ex1_1_hash_vs_bsearch
#
# Requires qemu-user-static to be installed on the host:
#   sudo apt-get install qemu-user-static       # Debian/Ubuntu
#   brew install qemu                           # macOS (Homebrew)
set -e

SYSROOT=/usr/riscv64-linux-gnu
QEMU=qemu-riscv64-static

if ! command -v "$QEMU" >/dev/null 2>&1; then
    echo "ERROR: $QEMU not found. Install qemu-user-static." >&2
    exit 1
fi

if [ $# -eq 0 ]; then
    # Run all built binaries
    for bin in build/riscv64/ch*/*; do
        [ -x "$bin" ] || continue
        echo "=== $bin ==="
        "$QEMU" -L "$SYSROOT" "$bin"
    done
else
    echo "=== $1 ==="
    "$QEMU" -L "$SYSROOT" "$1"
fi
