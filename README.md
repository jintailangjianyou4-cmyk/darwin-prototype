# Darwin Prototyping OS

This repository is a Darwin-inspired OS prototype designed to run inside a Linux devcontainer. It does not attempt to reproduce a complete Apple Darwin kernel; instead, it models a Darwin-like system architecture influenced by XNU, Mach, BSD, IOKit, and the boot/service flow of macOS.

Highlights:
- Darwin-like kernel boot sequence with Mach/BSD/IOKit references
- Process, filesystem, and service registry layers
- launchd-style service lifecycle
- CLI commands that mimic common Darwin/macOS tools
- Buildable with GCC on Linux and testable in a devcontainer

## Architecture

The prototype deliberately simulates the structure and behavior of Darwin without claiming to be a real kernel implementation.

- Kernel layer: `darwin_kernel` bootstraps the system and initializes core services.
- Process layer: minimal process state model for system tasks and user-space commands.
- Filesystem layer: a lightweight VFS-like abstraction using the local Linux filesystem under a Darwin state model.
- Service layer: launchd-like registry with services such as `com.apple.kernel`, `com.apple.launchd`, `com.apple.bsd`, and `com.apple.io`.
- IOKit-inspired layer: service registry and device-like object names represent hardware and driver boot concepts.

## Build

```bash
make
```

This will produce binaries in the `bin/` directory such as:

- `bin/darwin_kernel`
- `bin/uname`
- `bin/sw_vers`
- `bin/sysctl`
- `bin/launchctl`
- `bin/hostname`
- `bin/pwd`
- `bin/whoami`
- `bin/ls`
- `bin/help`
- `bin/shutdown`

## Run

```bash
./bin/darwin_kernel
./bin/uname
./bin/sw_vers
./bin/sysctl
./bin/launchctl list
./bin/hostname
./bin/pwd
./bin/whoami
./bin/ls
./bin/help
./bin/shutdown
```

## Expected output

```bash
$ ./bin/uname
Darwin 27.0.0
```

```bash
$ ./bin/sw_vers
ProductName: Darwin
ProductVersion: 27.0.0
BuildVersion: 27A
```

```bash
$ ./bin/sysctl | head
kern.ostype: Darwin
kern.osrelease: 27.0.0
kern.osrevision: 1
hw.machine: x86_64
```

```bash
$ ./bin/launchctl list
PID  Status  Label
0    running com.apple.kernel
1    running com.apple.launchd
2    running com.apple.bsd
3    stopped com.apple.io
4    stopped com.apple.apfs
```

## Testing

```bash
make test
```

The test script validates the most important outputs for the Darwin prototype and confirms that the boot and command layer remain consistent.

## Notes

- This project is intentionally not a full Darwin/KEXT/XNU reproduction.
- It follows the Darwin design philosophy and exposes a compatible CLI surface for experiments, teaching, and devcontainer-based workflows.
- The runtime state is stored in `/tmp/darwin_runtime_state` by default and can be overridden with `DARWIN_STATE_FILE`.
