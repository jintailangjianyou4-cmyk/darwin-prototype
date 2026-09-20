#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
export DARWIN_STATE_FILE="${TMPDIR:-/tmp}/darwin-prototype-test.state"
rm -f "$DARWIN_STATE_FILE"
make -C "$ROOT" all >/dev/null
"$ROOT/bin/darwin_kernel" >/dev/null
test "$("$ROOT/bin/uname")" = "Darwin 27.0.0"
grep -q 'ProductVersion: 27.0.0' <("$ROOT/bin/sw_vers")
grep -q 'kern.osrelease: 27.0.0' <("$ROOT/bin/sysctl" -a)
grep -q 'com.apple.launchd' <("$ROOT/bin/launchctl" list)
"$ROOT/bin/launchctl" start com.example.darwin-observer
grep -q 'running' <("$ROOT/bin/launchctl" list)
"$ROOT/bin/shutdown" >/dev/null
test -n "$("$ROOT/bin/hostname")"
test "$("$ROOT/bin/whoami")" = "root"
echo "Darwin prototype checks passed."
