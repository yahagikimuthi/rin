# rin

`rin` は Modern C++ (C++26) で書かれた、2Dゲーム開発向けの軽量なゲームエンジンです。

![Breakout Demo](examples/breakout/breakout.gif)


## 特徴

- **C++26 対応**: `std::expected` などを活用した例外機構を使わないモダンなエラーハンドリング.
- **FetchContent 対応**: CMake から簡単にプロジェクトへ組み込み可能.
- **2D レンダリング機能**: 頂点バッファ描画、スプライト、テキスト表示、カメラ制御.
- **標準ライブラリと同じ使用感**: クラス及び関数をsnake_caseで命名, 名前空間のネストを最小化.


## 動作環境

- **C++ コンパイラ**: C++26 サポート（GCC 16 等）
- **CMake**: 3.30 以上
- **WSL2**: 推奨環境（必要なOpenGLやGPU等の依存関係が.devcontainer/にセットアップ済み）


## 導入方法

自身の `CMakeLists.txt` に以下を追加してください。

```cmake
include(FetchContent)

FetchContent_Declare(
    rin
    GIT_REPOSITORY https://github.com/yahagikimuthi/rin.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(rin)

# 自身のターゲットにリンク
target_link_libraries(MyProject PRIVATE rin::rin_lib)
```


## サンプルコード (Examples)

リポジトリ内の `examples/` ディレクトリに、上記のブロック崩しをはじめとするサンプルコードを公開しています。


## 使用するサードパーティライブラリ

このゲームエンジンは以下のサードパーティライブラリをヘッダーファイルにおいてインクルードします。

- **glm**: 行列演算ライブラリ. `rin::vec2`などは`glm::vec2`との変換関数を用意
- **glad, KHR**: OpenGLを使用するために使用しています. 
- **miniaudio**: 音源を利用するため`audio_engine`クラスなどが使用します.
- **stb**: テクスチャ及びそれに基づくスプライト,フォント,文字列の利用のため使用します.

**注意**
glad, KHR, miniaudio, stbライブラリはC言語で記述されているため、**グローバル名前空間を汚染している**ことに注意してください。
さらに、gladはマクロ関数を多様しているため**glから始まるシンボル**に十分注意してください。