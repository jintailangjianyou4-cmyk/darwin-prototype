#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export PATH="$ROOT_DIR/bin:$PATH"

make -C "$ROOT_DIR" all >/dev/null

"$ROOT_DIR/bin/darwin_kernel" >/dev/null

UNAME_OUT="$("$ROOT_DIR/bin/uname")"
if [[ "$UNAME_OUT" != "Darwin 27.0.0" ]]; then
  echo "uname check failed: $UNAME_OUT" >&2
  exit 1
fi

HOSTNAME_OUT="$("$ROOT_DIR/bin/hostname")"
if [[ "$HOSTNAME_OUT" != "darwin-prototype" ]]; then
  echo "hostname check failed: $HOSTNAME_OUT" >&2
  exit 1
fi

SW_VERS_OUT="$("$ROOT_DIR/bin/sw_vers")"
if ! grep -q "ProductVersion: 27.0.0" <<< "$SW_VERS_OUT"; then
  echo "sw_vers check failed: $SW_VERS_OUT" >&2
  exit 1
fi

SYSCTL_OUT="$("$ROOT_DIR/bin/sysctl")"
if ! grep -q "kern.osrelease: 27.0.0" <<< "$SYSCTL_OUT"; then
  echo "sysctl check failed: $SYSCTL_OUT" >&2
  exit 1
fi

LAUNCHCTL_OUT="$("$ROOT_DIR/bin/launchctl" list)"
if ! grep -q "com.apple.launchd" <<< "$LAUNCHCTL_OUT"; then
  echo "launchctl check failed: $LAUNCHCTL_OUT" >&2
  exit 1
fi

WHOAMI_OUT="$("$ROOT_DIR/bin/whoami")"
if [[ "$WHOAMI_OUT" != "root" ]]; then
  echo "whoami check failed: $WHOAMI_OUT" >&2
  exit 1
fi

echo "Darwin prototype checks passed."
