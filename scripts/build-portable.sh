#!/usr/bin/env sh
set -eu
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/darwinctl boot
./build/darwinctl uname
