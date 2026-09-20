# Cross-platform build

The prototype now has a portable CMake target in addition to the original Linux `make` workflow. It builds the same Darwin observer on:

- Linux
- macOS
- BSD family systems with a C11 compiler
- Android through the Android NDK / Termux compiler
- Windows through MSVC, clang-cl, or MinGW

This is **not** a native Darwin kernel and does not replace the host OS. Every target runs the same user-space simulation. The platform adapter only changes safe host operations such as hostname lookup and directory enumeration.

## CMake build (recommended)

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run it with:

```sh
./build/darwinctl boot       # Windows: build\\darwinctl.exe boot
auto
./build/darwinctl uname
./build/darwinctl sw_vers
./build/darwinctl sysctl
./build/darwinctl launchctl
./build/darwinctl hostname
./build/darwinctl ls .
./build/darwinctl shutdown
```

The `darwinctl` command returns `Darwin 27.0.0` consistently on every supported host.

## Platform notes

### Linux, macOS, and BSD

Install a C11 compiler and CMake, then use the commands above. POSIX `dirent` is used for the portable `ls` behavior.

### Android

With the Android NDK:

```sh
cmake -S . -B build-android \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24
cmake --build build-android
```

On Termux, `clang` and `cmake` are sufficient for a native build.

### Windows

With Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The Windows adapter uses Win32 directory and hostname APIs only in the portability layer; the architecture and command behavior remain Darwin-inspired.

## Compatibility boundary

The project supports **building and observing the simulation** on these hosts. It does not claim binary compatibility with XNU, Linux, Android, Windows NT, or BSD kernels, and it does not attempt to expose privileged kernel APIs. The original Linux `make` targets remain available where their POSIX dependencies are present; `darwinctl` is the portable entry point.
