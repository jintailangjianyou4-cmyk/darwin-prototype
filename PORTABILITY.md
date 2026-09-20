# Portability matrix

The project now separates the **Darwin model** from optional host integration. This makes it possible to build the observer on environments beyond Linux, macOS, BSD, Android, and Windows.

## Supported host classes

| Host class | Mode | Host filesystem/hostname |
|---|---|---|
| POSIX systems | default | enabled |
| Win32 | default | enabled |
| Haiku, illumos, Solaris, QNX | default when C11/POSIX libc is available | usually enabled |
| Cygwin, MSYS2, WSL, containers, CI sandboxes | default | enabled through their exposed libc |
| WASI/WebAssembly | `DARWIN_PORTABLE_NO_HOST_APIS=ON` | disabled/fallback |
| freestanding or embedded C11 environments | `DARWIN_PORTABLE_NO_HOST_APIS=ON` | disabled/fallback |
| custom RTOS/libc ports | generic mode plus a platform adapter | port-defined |

The fallback mode only requires a C11 compiler, the standard C library, and stdout/stderr. It does not include `dirent.h`, `unistd.h`, or Win32 headers, so it is suitable for restricted SDKs, WebAssembly, firmware-like test harnesses, and unusual operating systems.

## Generic/no-host-API build

```sh
cmake -S . -B build-generic -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build build-generic
./build-generic/darwinctl boot
./build-generic/darwinctl uname
./build-generic/darwinctl ls .
```

In this mode `hostname` returns `darwin-prototype` and `ls` reports that the host filesystem is unavailable instead of depending on a platform API.

## WebAssembly/WASI example

With a WASI SDK installed:

```sh
cmake -S . -B build-wasi \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_C_FLAGS="--target=wasm32-wasi" \
  -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build build-wasi
```

The exact linker/runtime flags depend on the WASI SDK and runtime (`wasmtime`, `wasmer`, or another host), so the repository intentionally does not assume one specific SDK.

## Adding a new environment

1. Compile in generic mode first.
2. If the environment provides POSIX directory and hostname APIs, use the default mode.
3. Otherwise define `DARWIN_PORTABLE_NO_HOST_APIS` and provide a small adapter in `src/portable_runtime.c` for any native services you want.
4. Keep native calls inside the adapter; the command and simulated kernel layers remain unchanged.

This expands **build and observation portability**. It does not provide native kernel integration or claim that every target can boot real Darwin/XNU.
