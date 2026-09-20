# Darwin Prototype

Darwin 27.0.0風のOS観測・対話プロトタイプです。実際のXNUカーネルではなく、ユーザー空間でMach/BSD/IOKit/launchdを模倣します。

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

ホストAPIを使わない汎用モード:

```sh
cmake -S . -B build-generic -DDARWIN_PORTABLE_NO_HOST_APIS=ON
cmake --build build-generic
```

## Interactive shell

```sh
./build/darwin-shell
```

主なコマンド:

```text
boot / shutdown
ps / top
sysctl -a
kextstat
launchctl list
service status com.apple.launchd
spawn worker
mach ports
mach send 100 hello
iokit tree
dmesg
ls / hostname pwd whoami
```

Shellは仮想process table、Mach port風IPC、IOKitデバイスツリー、boot/kernel logをメモリ上で管理します。`spawn`や`mach send`を実行すると状態とログが変化します。

## Cross-platform adapters

`src/darwin_adapter.c`にホスト依存処理を集約しています。POSIX、Windows、Android、macOS、BSD、Haiku、illumos/Solaris、QNXを判定し、未知の環境やWASI・組み込みではgeneric C11 adapterへフォールバックします。

## launchd風設定

`config/com.example.darwin-observer.plist`にplist風のサービス定義を収録しています。現在は表示・設計用で、実ホストのlaunchdへ登録するものではありません。

## 注意

このプロジェクトはOSの観察教材・シミュレータです。ホストOSのカーネルを置き換えず、特権APIや実際のプロセスを制御しません。
