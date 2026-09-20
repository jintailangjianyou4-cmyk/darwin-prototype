#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR=${BUILD_DIR:-"$ROOT/build"}

cmake -S "$ROOT" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"
ctest --test-dir "$BUILD_DIR" --output-on-failure

SHELL_BIN="$BUILD_DIR/darwin-shell"
[ -x "$SHELL_BIN" ]

TRANSCRIPT=$(mktemp)
trap 'rm -f "$TRANSCRIPT"' EXIT
cat <<'EOF' | "$SHELL_BIN" >"$TRANSCRIPT"
boot
launchctl list
launchctl start com.example.darwin-observer
launchctl print com.example.darwin-observer
sysctl -a
log show --predicate com.apple.launchd
shutdown
log show --last boot
exit
EOF

grep -q 'Darwin Shell 27.0.0' "$TRANSCRIPT"
grep -q 'com.example.darwin-observer' "$TRANSCRIPT"
grep -q 'kern.osrelease: 27.0.0' "$TRANSCRIPT"
grep -q 'com.apple.launchd' "$TRANSCRIPT"
grep -q 'kernel halt requested' "$TRANSCRIPT"

echo "Darwin shell integration checks passed."
