# 開発・ビルド環境ガイド (`BUILD_ENVIRONMENT.md`)

本ライブラリ（`rin`）の推奨開発環境および Dev Container（Docker）でのセットアップ手順について解説します。

## 推奨開発環境

本ライブラリのプライマリ開発・動作確認環境は以下の通りです。

- **OS**: Windows 11 (WSL2 / WSLg)
- **開発環境**: VS Code + Dev Containers プラグイン
- **コンパイラ**: GCC 16.1 (C++26 一部機能のサポート必須)

> **Note**  
> 開発者の主環境が WSL2 + Dev Container であるため、クロスプラットフォーム対応および環境差分による動作のサポートは現在制限されています。原則として以下のDev Container構成での動作を推奨します。

---

## Dev Container 設定ガイド

WSLg（GUI / オーディオパススルー）および GPU アクセス（Direct3D12 / Mesa 経由）を正常に機能させるため、`.devcontainer/devcontainer.json` に以下の設定を組み込むことを推奨します。

### 1. 環境変数 (`containerEnv`)

OpenGLコンテキスト生成および PulseAudio による音声出力を有効化するための推奨設定です。

```json
"containerEnv": {
    "DISPLAY": ":0",
    "WAYLAND_DISPLAY": "wayland-0",
    "XDG_RUNTIME_DIR": "/mnt/wslg/runtime-dir",
    "MESA_LOADER_DRIVER_OVERRIDE": "d3d12",
    "GALLIUM_DRIVER": "d3d12",
    "LD_LIBRARY_PATH": "/usr/lib/wsl/lib",
    "MESA_GL_VERSION_OVERRIDE": "4.5COMPAT",
    "MESA_GLSL_VERSION_OVERRIDE": "450",
    "PULSE_SERVER": "unix:/mnt/wslg/PulseServer"
}
```

### 2. コンテナ実行引数(`runArgs`)

ホスト側(WSLg/GPUドライバ)のデバイスおよびソケットをコンテナにパススルーするための推奨マウント設定です。

```json
"runArgs": [
    "--device=/dev/dxg",
    "-v", "/tmp/.X11-unix:/tmp/.X11-unix",
    "-v", "/mnt/wslg:/mnt/wslg",
    "-v", "/usr/lib/wsl:/usr/lib/wsl",
    "--ipc=host"
]
```

### 3. コンテナセットアップ(`postCreateCommand`)

Dev Container の初回起動時に実行されるパッケージのセットアップスクリプトと、各依存関係の導入理由です。

```bash
set -e

# パッケージリポジトリの追加準備
sudo apt-get update
sudo apt-get install -y software-properties-common wget gpg ca-certificates

# Toolchain PPA(GCC 16 取得用)の追加
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

# Kitware 公式リポジトリ(最新版CMake取得用)の追加
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

# 開発ツール・ビルドチェーン・描画依存ライブラリのインストール
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
    gcc-16 \
    g++-16 \
    cmake \
    ninja-build \
    gdb \
    lldb \
    libglfw3-dev \
    libglm-dev \
    libgl1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev

# デフォルトの C/C++ コンパイラを GCC 16 に切り替え
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-16 \
  --slave /usr/bin/gcov gcov /usr/bin/gcov-16

# OpenGL / Mesa ユーティリティ・ドライバのインストール
sudo apt-get update && sudo apt-get install -y mesa-utils libgl1-mesa-dri libgl1 libglx-mesa0 libglfw3-dev

# オーディオ依存ライブラリ(ALSA / PulseAudio)のインストール
sudo apt-get update && sudo apt-get install -y \
    libasound2-dev \
    libpulse-dev \
    alsa-utils \
    pulseaudio-utils
```

#### パッケージ群の導入理由・詳細

| パッケージ・リポジトリ | 導入理由・役割 |
| :--- | :--- |
| **`ubuntu-toolchain-r/test` (PPA) / `gcc-16`, `g++-16`** | 本ライブラリは **C++26** の最新言語仕様を採用しているため、高度な C++26 機能をサポートする **GCC 16** を明示的に指定してインストール・デフォルト化しています。 |
| **`apt.kitware.com` / `cmake`** | 最新の C++26 機能や依存ライブラリ（`FetchContent` 等）のビルド制御に対応するため、Kitware 公式の最新版 CMake を取得しています。 |
| **`ninja-build`** | 高速な並列ビルドを行うためのビルドシステムです。 |
| **`libglfw3-dev`, `libglm-dev`** | ウィンドウ管理・入力処理を行う **GLFW** と、ベクトル・行列演算ライブラリ **GLM** の開発用ヘッダーです。 |
| **`libgl1-mesa-dev`, `libx11-dev` 等** | Linux (X11/GLX) 環境上で OpenGL ウインドウおよび入力・グラフィックコンテキストを生成するためのシステム依存ヘッダー群です。 |
| **`mesa-utils`, `libgl1-mesa-dri` 等** | WSLg / Direct3D12 パススルー経由で ハードウェア GPU アクセラレーション（Mesa）を動作させるためのドライバおよび診断ツール（`glxinfo` 等）です。 |
| **`libasound2-dev`, `libpulse-dev`** | **`miniaudio`** が Linux / WSLg 環境のサウンドサーバー（PulseAudio / ALSA）と通信して音声を出力するために必要なヘッダーおよび共有ライブラリです。 |
| **`alsa-utils`, `pulseaudio-utils`** | 音声デバイスの認識確認やテスト再生（`aplay`, `paplay` 等）を行うための診断ツール群です。 |

---
## 注意点

### サニタイザ(AddressSanitizer)使用時の注意点

本ライブラリ(`rin`)のコードベース自体はメモリ安全を考慮していますが、ご利用のグラフィックドライバ（Mesa/GLX等）や環境によっては、**AddressSanitizer(ASan)を有効にした際にOpenGLコンテキストの生成に失敗する**既知の非互換問題が存在します。

#### 推奨される対処・利用方法

1. **通常開発時(Debugビルド)**
   グラフィックス描画やウィンドウ処理の動作確認時は、AddressSanitizerをオフ(または`-fsanitize=undefined`等のUBSanのみ)にしてビルドすることを推奨します。
2. **メモリ検証時**
   フォントの参照を持つテキストクラスなど、メモリのチェックを行う場合は、ロジック単位でASanを有効化することを強く推奨します。

### オーディオ構成

`miniaudio`はPulseAudio(WSLgのPulseServer)を経由して音声を再生します。
コンテナ内でaplay -lなどのALSA物理デバイス検索を実行するとサウンドカード未検出と表示される場合がありますが、PULSE_SERVER経由での音声再生は正常に機能します。

### 例外

本ライブラリでは例外を使用しておらず、すべての関数にnoexceptを付与し、std::expectedを用いたエラーハンドルを行っています。そのため、-fno-exceptions指定によるコンパイルが可能です。
ただし、標準ライブラリ等から投げられる例外(メモリ不足など)をハンドルできず、直ちにstd::terminateが呼ばれることに注意してください。

### 名前空間等

1. 本ライブラリのソースコードでは、サードパーティライブラリを除いて2026/10/3現在においてマクロを使用していませんが、RIN_から始まるマクロを使用者に対して予約するものとします。

2. 私が記述したマクロを除く全てのシンボルはrin名前空間に配置されています。ただし、サードパーティライブラリである、glad, KHR, GLFW, miniaudio, stbライブラリはC言語で記述されています。これらを本ライブラリはヘッダーファイルにおいてインクルードしているため、**グローバル名前空間を汚染している**ことに注意してください。
さらに、gladはマクロ関数を多様しているため**glから始まるシンボル**に十分注意してください。