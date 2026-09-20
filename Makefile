CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude
BIN_DIR = bin

COMMANDS = $(BIN_DIR)/darwin_kernel \
	$(BIN_DIR)/uname \
	$(BIN_DIR)/sw_vers \
	$(BIN_DIR)/sysctl \
	$(BIN_DIR)/launchctl \
	$(BIN_DIR)/hostname \
	$(BIN_DIR)/pwd \
	$(BIN_DIR)/whoami \
	$(BIN_DIR)/ls \
	$(BIN_DIR)/help \
	$(BIN_DIR)/shutdown

all: $(COMMANDS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/darwin_kernel: src/darwin_kernel.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/darwin_kernel.c src/darwin_runtime.c

$(BIN_DIR)/uname: src/uname.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/uname.c src/darwin_runtime.c

$(BIN_DIR)/sw_vers: src/sw_vers.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/sw_vers.c src/darwin_runtime.c

$(BIN_DIR)/sysctl: src/sysctl.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/sysctl.c src/darwin_runtime.c

$(BIN_DIR)/launchctl: src/launchctl.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/launchctl.c src/darwin_runtime.c

$(BIN_DIR)/hostname: src/hostname.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/hostname.c src/darwin_runtime.c

$(BIN_DIR)/pwd: src/pwd.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/pwd.c src/darwin_runtime.c

$(BIN_DIR)/whoami: src/whoami.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/whoami.c src/darwin_runtime.c

$(BIN_DIR)/ls: src/ls.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/ls.c src/darwin_runtime.c

$(BIN_DIR)/help: src/help.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/help.c src/darwin_runtime.c

$(BIN_DIR)/shutdown: src/shutdown.c src/darwin_runtime.c include/darwin_os.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/shutdown.c src/darwin_runtime.c

check: all
	./tests/run_checks.sh

test: check

clean:
	rm -rf $(BIN_DIR)
