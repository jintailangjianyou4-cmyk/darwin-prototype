# Darwin Prototype

Darwin 27.0.0-inspired OS observation and interactive shell prototype. It is not the XNU kernel; it is a user-space simulation of selected Mach, BSD, IOKit, and launchd concepts.

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

Only `src/darwin_adapter.c` uses POSIX or Win32 host APIs in full mode. The simulated Darwin core remains host-independent.

## Darwin-like shell commands

```sh
./build/darwin-shell
```

Supported commands include:

```text
boot | reboot | shutdown
ps [-axo]
top [-o cpu|mem]
sysctl -a
sysctl kern.osrelease
kextstat
launchctl list
launchctl print com.apple.launchd
launchctl start com.example.darwin-observer
launchctl stop com.example.darwin-observer
ioreg -l
iokit tree
dmesg
log show --last boot
mach ports
mach send 100 hello
spawn worker
kill 104
```

`sysctl -a` prints a structured kernel/hardware/VFS inventory. `kextstat` shows simulated kernel extensions. `launchctl print` reports service state, PID, program, run count, and keep-alive policy. `ioreg` and `iokit tree` expose the virtual I/O registry.

## Realistic boot and shutdown event flow

The observer records an ordered kernel-style event chain:

1. Kernel handoff and Mach IPC initialization
2. BSD VM, credentials, and VFS initialization
3. Platform expert matching the root device
4. IOKit registry publication
5. Virtual root filesystem mount
6. launchd bootstrap namespace creation
7. launchd service set loading
8. Multi-user userspace readiness

Shutdown records the reverse lifecycle: halt request, user-service drain in reverse order, virtual filesystem unmount, Mach port drain, and kernel halt request. Use `dmesg` or `log show --last boot` after each operation to inspect the chain.

## Architecture

- `src/darwin_observer.c`: host-independent Darwin-like kernel/process/service/IPC/device model
- `src/darwin_shell.c`: interactive Darwin/macOS-style command interpreter
- `src/darwin_adapter.c`: isolated hostname, working-directory, and directory-listing adapters
- Full mode: POSIX/Win32 APIs through the adapter layer
- Minimal mode: standard C plus stdout/stderr, with no host filesystem or hostname APIs
- `config/com.example.darwin-observer.plist`: launchd-style service definition

This provides portable observation of a simulation on Linux, macOS, BSD, Android, Windows, Haiku, illumos/Solaris, QNX, Cygwin/MSYS2/WSL, WASI, embedded C11 environments, and other systems with a suitable C compiler. It does not provide native kernel compatibility.
