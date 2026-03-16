ARCH        ?= riscv64
BUILD_DIR   := build/$(ARCH)

# ── Toolchain selection ──────────────────────────────────────────────────────
ifeq ($(ARCH),riscv64)
  CC      := riscv64-linux-gnu-gcc
  CFLAGS  := -march=rv64gc -mabi=lp64d -O2 -Wall -Wextra -g
  RUNNER  := qemu-riscv64-static -L /usr/riscv64-linux-gnu
else ifeq ($(ARCH),x86_64)
  CC      := gcc
  CFLAGS  := -O2 -Wall -Wextra -g
  RUNNER  :=
else
  $(error Unsupported ARCH=$(ARCH). Use riscv64 or x86_64)
endif

COMMON_DIR  := exercises/common
COMMON_SRC  := $(COMMON_DIR)/benchmark.c
COMMON_INC  := -I$(COMMON_DIR)

# ── Discover all exercise sources ────────────────────────────────────────────
# Each exercises/chNN/exNN_*.c compiles to its own binary
EX_SRCS := $(wildcard exercises/ch*/ex*.c)
EX_BINS := $(patsubst exercises/%.c,$(BUILD_DIR)/%,$(EX_SRCS))

.PHONY: all run clean

all: $(EX_BINS)

# Pattern rule: compile exercise + common benchmark.c into one binary
$(BUILD_DIR)/%: exercises/%.c $(COMMON_SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(COMMON_INC) $^ -o $@ -lpthread -lm

run: all
	@echo "=== Running all exercises under ARCH=$(ARCH) ==="
	@for bin in $(EX_BINS); do \
	    echo "--- $$bin ---"; \
	    $(RUNNER) $$bin || exit 1; \
	done

clean:
	rm -rf build/
