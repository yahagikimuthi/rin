#include "rin/texture.hpp"
#include "rin/window.hpp"

/**
 * @brief textureの描画
 * @details 画面の中央にテクスチャを描画する
 * @warning
 * texture.pngのパスに注意してください。実行する場合は02_texture直下に実行ファイルを置いてください。
 */
auto main() -> int {
    auto win_res = rin::try_make_window(800, 600);
    if (not win_res) win_res.error().panic();
    auto& win = *win_res;

    // テクスチャの生成, 失敗した場合は異常終了
    auto texture_res = rin::try_make_texture("texture.png");
    if (not texture_res) texture_res.error().panic();
    auto& texture = *texture_res;

    // スプライトの生成
    auto sprite = rin::make_sprite(texture);

    // サイズを取得し、重心(基準点)を設定
    const auto size = sprite.size();
    sprite.origin(size.width / 2, size.height / 2);

    // ウィンドウの中心点に表示
    sprite.position(400, 300);

    while (win.is_open()) {
        win.poll_events();
        win.begin_render();
        win.draw(sprite);
        win.end_render();
    }
}