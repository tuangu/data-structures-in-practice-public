FROM ubuntu:24.04

# Prevent interactive prompts during apt install
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-riscv64-linux-gnu \
    binutils-riscv64-linux-gnu \
    qemu-user-static \
    libc6-dev-riscv64-cross \
    gcc \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Verify the cross toolchain and QEMU are present
RUN riscv64-linux-gnu-gcc --version && qemu-riscv64-static --version

WORKDIR /sandbox

# Default: build all exercises for RISC-V
CMD ["make", "ARCH=riscv64", "all"]
