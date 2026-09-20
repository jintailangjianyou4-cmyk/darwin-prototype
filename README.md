# Darwin Prototype

Darwin 27.0.0風のOS観測・対話プロトタイプです。実際のXNUカーネルではなく、ユーザー空間でMach/BSD/IOKit/launchdを模倣します。

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

汎用minimal mode（標準C + stdout/stderrのみ）:

```sh
cmake -S . -B build-generic -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build build-generic
```

通常のfull modeでは、`src/darwin_adapter.c`だけがPOSIX/Win32 APIを使用し、共通コアはホスト非依存です。

## darwin-shell

```sh
./build/darwin-shell
```

Darwin系の入力に寄せたコマンド:

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
dmesg
log show
```

`ps`はプロセス一覧、`top`はCPU/メモリ列を含む上位表示として出力形式を分けています。`launchctl list`はPID、状態、labelを表示し、`print`はprogram、runs、keepaliveまで表示します。

## 状態とログ

シェル内の仮想process table、launchdサービス、Mach port、IOKitツリーは同じ共通コア状態を参照します。`boot`、`spawn`、`kill`、`mach send`、`launchctl start/stop`、`shutdown`がkernel logを追加するため、`dmesg`または`log show`でイベントを追跡できます。

## Adapter設計

- `darwin_observer.c`: Darwin構造を模倣した共通コア
- `darwin_adapter.c`: hostname、cwd、directory listingだけをOS APIへ接続
- full mode: POSIX/Win32 APIを使用
- minimal mode: `DARWIN_PORTABLE_NO_HOST_APIS=ON`で標準Cとstdout/stderrだけ

対応しやすい環境はLinux、macOS、BSD、Android、Windowsに加えて、Haiku、illumos/Solaris、QNX、Cygwin/MSYS2/WSL、WASI、組み込みC11環境です。
