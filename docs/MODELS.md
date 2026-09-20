# Darwin Prototype Model

This document defines the three models used by the Darwin-inspired observer.

## Service state machine

The simulated launchd service lifecycle is intentionally explicit:

```text
stopped --load--> loading --successful start--> running
running --stop--> stopping --drain complete--> stopped
running --crash--> waiting --retry--> loading
loading --failure--> waiting
waiting --disable--> stopped
```

The shell exposes the safe transitions through `launchctl`:

- `launchctl list`
- `launchctl print <label>`
- `launchctl start <label>`
- `launchctl stop <label>`
- `launchctl kickstart <label>`

Every accepted transition updates the shared service table and emits one ordered `com.apple.launchd` event. Repeating an idempotent operation does not generate a false transition.

## sysctl inventory

The inventory is grouped like a Darwin diagnostic session:

- `kern.*`: OS identity, kernel version, hostname, boot time, process and file limits
- `kern.ipc.*`: Mach port and IPC limits
- `kern.vm.*`: page size and virtual memory values
- `hw.*`: machine identity, CPU count, memory size, byte order, 64-bit capability
- `vfs.*`: virtual root filesystem and filesystem limits

`sysctl -a` prints the complete static inventory. `sysctl <oid>` prints one exact key and returns an error for unknown keys.

## Ordered log model

Each event has:

- a monotonically increasing sequence number
- a synthetic timestamp
- severity
- subsystem
- category
- message

The event store is append-only for the lifetime of a shell session. `dmesg` prints all events. `log show` prints the same events with optional filters:

```text
log show --last boot
log show --predicate com.apple.launchd
log show --level NOTICE
```

Boot events are ordered from kernel handoff through Mach, BSD, IOKit, filesystem mount, launchd bootstrap, service loading, and multi-user readiness. Shutdown reverses those layers: halt request, service drain, filesystem unmount, Mach-port drain, and kernel halt.

All three models are part of the user-space simulation and do not control the host kernel.
