# rin

`rin` は Modern C++ (C++26) で書かれた、2Dゲーム開発向けの軽量なヘッダーオンリー・ゲームエンジンです。

![Breakout Demo](examples/breakout/breakout.gif)

## 特徴

- **C++26 対応**: `std::expected` などを活用したモダンなエラーハンドリング
- **FetchContent 対応**: CMake から簡単にプロジェクトへ組み込み可能
- **2D レンダリング機能**: 頂点バッファ描画、スプライト、テキスト表示、カメラ制御

## 動作環境

- **C++ コンパイラ**: C++26 サポート（GCC 14+ / Clang 18+ 等）
- **CMake**: 3.30 以上
- **WSL2**: 推奨環境（必要なOpenGLやGPU等の依存関係が.devcontainer/にセットアップ済み）

## 導入方法

自身の `CMakeLists.txt` に以下を追加してください。

```cmake
include(FetchContent)

FetchContent_Declare(
    rin
    GIT_REPOSITORY [https://github.com/your_username/rin.git](https://github.com/your_username/rin.git)
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(rin)

# 自身のターゲットにリンク
target_link_libraries(my_game PRIVATE rin::rin_lib)
```

## サンプルコード (Examples)

リポジトリ内の `examples/` ディレクトリに、上記のブロック崩しをはじめとするサンプルコードを公開しています。

- **[Breakout (ブロック崩し)](examples/breakout/main.cpp)**: `rin` の描画ルーチン・入力処理・衝突判定・オーディオを用いた簡単な実装例