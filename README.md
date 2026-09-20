# Darwin Prototype

Darwin 27.0.0-inspired OS observation and interactive shell prototype. It is not the XNU kernel; it is a user-space simulation of selected Mach, BSD, IOKit, and launchd concepts.

## Recent command fidelity improvements

- `launchctl list` is backed by the same service state used by `launchctl print`, `start`, and `stop`.
- Repeated `start`/`stop` operations are idempotent and do not create false lifecycle events.
- Service lifecycle changes emit `com.apple.launchd` log records.
- `sysctl -a` exposes a larger macOS-style inventory of `kern.*`, `hw.*`, and `vfs.*` keys.
- `log show` prints ordered sequence numbers, severity, subsystem, category, and message.
- `log show --predicate com.apple.launchd` filters the event stream by subsystem.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Generic minimal mode:

```sh
cmake -S . -B build-generic -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build build-generic
```

## Shell examples

```text
boot
sysctl -a
kextstat
launchctl list
launchctl print com.apple.launchd
launchctl start com.example.darwin-observer
launchctl stop com.example.darwin-observer
log show --predicate com.apple.launchd
log show --last boot
dmesg
ioreg -l
shutdown
```

## Event model

Boot events are recorded in realistic order: kernel handoff, Mach initialization, BSD VM/VFS setup, platform matching, IOKit registry publication, root filesystem mount, launchd bootstrap, service loading, and multi-user readiness.

Shutdown follows the reverse lifecycle: halt request, launchd service drain, virtual filesystem unmount, Mach port drain, and kernel halt request. Both `dmesg` and `log show` read the same ordered event store, so service state and displayed logs remain consistent.

## Architecture

- `src/darwin_observer.c`: host-independent Darwin-like kernel/process/service/IPC/device model and ordered event store
- `src/darwin_shell.c`: interactive Darwin/macOS-style command interpreter
- `src/darwin_adapter.c`: isolated hostname, working-directory, and directory-listing adapters
- Full mode: POSIX/Win32 APIs through the adapter layer
- Minimal mode: standard C plus stdout/stderr, with no host filesystem or hostname APIs
- `config/com.example.darwin-observer.plist`: launchd-style service definition

This provides portable observation of a simulation on Linux, macOS, BSD, Android, Windows, Haiku, illumos/Solaris, QNX, Cygwin/MSYS2/WSL, WASI, embedded C11 environments, and other systems with a suitable C compiler. It does not provide native kernel compatibility.
