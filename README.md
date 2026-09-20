# Darwin Prototype

Darwin 27.0.0-inspired OS observation and interactive shell prototype. It is not the XNU kernel; it is a user-space simulation of selected Mach, BSD, IOKit, and launchd concepts.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

For the generic minimal mode, which uses only standard C and stdout/stderr:

```sh
cmake -S . -B build-generic -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build build-generic
```

In full mode, only `src/darwin_adapter.c` uses POSIX or Win32 host APIs. The simulated Darwin core remains host-independent.

## Interactive shell

```sh
./build/darwin-shell
```

The shell accepts Darwin/macOS-inspired commands:

```text
boot | reboot | shutdown
ps [-axo]
top [-o cpu|mem]
sysctl -a
kextstat
launchctl list
launchctl print com.apple.launchd
launchctl start com.example.darwin-observer
launchctl stop com.example.darwin-observer
service status com.apple.launchd
spawn worker
kill 104
mach ports
mach send 100 hello
iokit tree
ioreg
dmesg
log show --last boot
ls / hostname pwd whoami
```

`ps` shows the complete virtual process table in a compact BSD-like format. `top` shows a sorted, abbreviated live-style view with CPU, memory, state, and command columns. `launchctl list` reports PID, state, and service label; `launchctl print` reports program, run count, and keep-alive policy.

## Event-driven kernel-style logs

The common observer core owns the virtual process table, launchd service table, Mach ports, IOKit registry, and kernel log. These operations append log records:

- `boot` and `reboot`: kernel, Mach, BSD, IOKit, and launchd bootstrap events
- `spawn` and `kill`: BSD process lifecycle events
- `mach send`: Mach message events
- `launchctl start/stop`: launchd service lifecycle events
- `shutdown`: service draining and kernel halt events

Use `dmesg` or `log show` to inspect the resulting event chain.

## Architecture

- `src/darwin_observer.c`: host-independent Darwin-like kernel/process/service/IPC/device model
- `src/darwin_shell.c`: interactive command interpreter
- `src/darwin_adapter.c`: isolated hostname, working-directory, and directory-listing adapters
- full mode: POSIX/Win32 APIs through the adapter layer
- minimal mode: standard C plus stdout/stderr, with no host filesystem or hostname APIs
- `config/com.example.darwin-observer.plist`: launchd-style service definition for observation and documentation

The design is intended to build on Linux, macOS, BSD, Android, Windows, Haiku, illumos/Solaris, QNX, Cygwin/MSYS2/WSL, WASI, embedded C11 environments, and other systems with a suitable C compiler. This means portable observation of the simulation, not native kernel compatibility.
