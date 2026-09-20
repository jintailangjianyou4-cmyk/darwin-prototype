CC ?= cc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic
BIN_DIR ?= bin
RUNTIME = src/darwin_runtime.c

COMMANDS = darwin_kernel uname sw_vers sysctl launchctl hostname pwd whoami ls help shutdown
BINS = $(COMMANDS:%=$(BIN_DIR)/%)

.PHONY: all clean test
all: $(BINS)

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/%: src/%.c $(RUNTIME) include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(RUNTIME)

test: all
	./tests/run_checks.sh

clean:
	rm -rf $(BIN_DIR)
