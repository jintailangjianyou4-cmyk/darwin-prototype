# Darwin 27.0.0 Prototype

This is a C-first, user-space operating-system prototype inspired by XNU, Mach, BSD, IOKit, launchd, and the Darwin boot flow. It does **not** reproduce the real Darwin kernel.

## Layers

- **Kernel-like layer:** deterministic boot and shutdown state transitions.
- **Process layer:** virtual service/PID model.
- **Filesystem layer:** stateful virtual working directory plus host-backed `ls`.
- **Service layer:** launchd-like service registry and state machine.
- **Command layer:** Darwin-style command names and output.

The primary implementation is C. Python is not required.

## Build on a Linux devcontainer

```sh
make
```

The binaries are written to `bin/`.

## Run

```sh
./bin/darwin_kernel
./bin/uname
./bin/sw_vers
./bin/sysctl -a
./bin/launchctl list
./bin/hostname
./bin/pwd
./bin/whoami
./bin/ls .
./bin/help
./bin/shutdown
```

Expected identity:

```text
Darwin 27.0.0
```

## Service state machine

The launchd-like model uses these states:

```text
stopped -> loading -> running
running -> stopping -> stopped
running -> waiting -> loading
```

Use:

```sh
./bin/launchctl list
./bin/launchctl print com.apple.launchd
./bin/launchctl start com.example.darwin-observer
./bin/launchctl stop com.example.darwin-observer
```

The service table is shared by all `launchctl` invocations through `DARWIN_STATE_FILE`. Repeated start/stop operations are idempotent.

## Sysctl inventory

`sysctl -a` provides Darwin-like groups:

- `kern.*`: identity, kernel version, limits, hostname
- `kern.ipc.*`: Mach port capacity
- `kern.vm.*`: page size
- `hw.*`: machine, model, CPU, memory, byte order
- `vfs.*`: virtual root filesystem

Individual keys can be queried with `sysctl kern.osrelease` or `sysctl hw.ncpu`.

## Tests

```sh
make test
# or
./tests/run_checks.sh
```

The test covers boot, `uname`, `sw_vers`, `sysctl`, service listing/start, shutdown, hostname, and user identity.

## State location

The default state file is `/tmp/darwin_prototype.state`. Override it for isolated runs:

```sh
DARWIN_STATE_FILE=/tmp/my-darwin.state ./bin/darwin_kernel
```

## Publish changes

```sh
git status
git add .
git commit -m "Implement Darwin 27.0.0 OS prototype"
git push origin main
```

For a feature branch:

```sh
git switch -c feature/darwin-prototype
git add .
git commit -m "Implement Darwin-inspired OS layers"
git push -u origin feature/darwin-prototype
```

This project observes a simulation in user space; it does not replace or control the host kernel.
