# Darwin Prototype

Darwin 27.0.0-inspired OS observation and interactive shell prototype. It is not the XNU kernel; it is a user-space simulation of selected XNU, Mach, BSD, IOKit, launchd, and boot-flow concepts.

## Architecture

- **Kernel-like layer**: ordered boot, shutdown, and kernel event flow
- **Mach-like layer**: virtual ports and message events
- **BSD-like layer**: virtual process table and lifecycle events
- **Filesystem layer**: virtual root filesystem inventory plus host adapter for `ls`, `pwd`, and `hostname`
- **IOKit-like layer**: virtual device registry exposed by `ioreg` and `iokit tree`
- **launchd-like service layer**: service state machine, plist metadata, PID/state reporting
- **Adapter layer**: isolated POSIX/Win32 APIs with a generic C11 fallback

The implementation is C-first. The host-independent observer core does not require Python.

## Service state machine

The service model uses explicit states and ordered transitions:

```text
stopped -> loading -> running
running -> stopping -> stopped
running -> waiting -> loading
loading -> waiting
waiting -> stopped
```

Use:

```text
launchctl list
launchctl print com.apple.launchd
launchctl start com.example.darwin-observer
launchctl stop com.example.darwin-observer
launchctl kickstart com.example.darwin-observer
```

State changes and service logs use the same in-memory model. Repeating `start` or `stop` when already in that state is idempotent.

## sysctl inventory and ordered logs

The shell provides macOS-style inventory groups:

```text
sysctl -a
sysctl kern.osrelease
sysctl hw.ncpu
```

Inventory includes `kern.*`, `kern.ipc.*`, `kern.vm.*`, `hw.*`, and `vfs.*` keys. Unknown OIDs are reported instead of silently returning a value.

The ordered event store records sequence, timestamp, level, subsystem, category, and message. Use:

```text
dmesg
log show --last boot
log show --predicate com.apple.launchd
log show --level NOTICE
```

Boot order is kernel handoff, Mach IPC, BSD VM/VFS, platform matching, IOKit registry, root filesystem mount, launchd bootstrap, service loading, and multi-user readiness. Shutdown drains services in reverse order, unmounts the virtual filesystem, drains Mach ports, and requests the kernel halt.

## Build on Linux devcontainers

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Generic minimal mode, with no host filesystem or hostname APIs:

```sh
cmake -S . -B build-generic -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build build-generic
ctest --test-dir build-generic --output-on-failure
```

## Run

```sh
./build/darwinctl uname
./build/darwinctl sw_vers
./build/darwinctl sysctl
./build/darwin-shell
```

Example shell session:

```text
$ ./build/darwin-shell
Darwin Shell 27.0.0 (simulated user space)
darwin% boot
boot complete
darwin% sysctl kern.osrelease
27.0.0
darwin% launchctl list
PID  Status   Label
0    running  com.apple.kernel
1    running  com.apple.launchd
...
darwin% log show --predicate com.apple.launchd
00:00:06 [0006] INFO   com.apple.launchd       bootstrap     bootstrap namespace created
...
darwin% shutdown
shutdown requested
```

## Tests

```sh
ctest --test-dir build --output-on-failure
sh tests/shell_transcript.sh
```

## Publish the project

The following commands build, test, commit, and publish the prototype to the configured Git remote:

```sh
git clone https://github.com/jintailangjianyou4-cmyk/darwin-prototype.git
cd darwin-prototype
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
git status
git add CMakeLists.txt README.md docs/MODELS.md tests/shell_transcript.sh
git commit -m "Model launchd states sysctl inventory and ordered logs"
git push origin main
```

For a new branch and pull request:

```sh
git switch -c feature/darwin-observer-models
git add .
git commit -m "Improve Darwin observer models"
git push -u origin feature/darwin-observer-models
# Then open a pull request for feature/darwin-observer-models -> main.
```

This project observes a simulation; it does not replace or control the host kernel.
