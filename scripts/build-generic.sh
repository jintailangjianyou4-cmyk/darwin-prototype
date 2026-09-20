#!/usr/bin/env sh
set -eu

BUILD_DIR="${1:-build-generic}"
cmake -S . -B "$BUILD_DIR" -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build "$BUILD_DIR"
ctest --test-dir "$BUILD_DIR" --output-on-failure
"$BUILD_DIR/darwinctl" boot
"$BUILD_DIR/darwinctl" uname
