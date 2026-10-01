#pragma once

/**
 * @file rin.hpp
 * @brief rin ゲームエンジンライブラリのメイン統合ヘッダーファイル
 *
 * @details このヘッダーファイルをインクルードすることで、rin ライブラリが提供する
 * すべての型、構造体、ウィンドウ管理、描画ルーチン、およびユーティリティ機能へアクセス可能になります。
 *
 * @code
 * #include <rin/rin.hpp>
 *
 * int main() {
 *     auto win_res = rin::try_make_window(800, 600, "Hello rin");
 *     if (!win_res) return win_res.error().panic();
 *     auto& win = *win_res;
 *
 *     while (win.is_open()) {
 *         win.poll_events();
 *         win.begin_render();
 *         // 描画処理
 *         win.end_render();
 *     }
 * }
 * @endcode
 */

#include "audio.hpp"
#include "clock.hpp"
#include "error.hpp"
#include "extent.hpp"
#include "font.hpp"
#include "key.hpp"
#include "mouse.hpp"
#include "sprite.hpp"
#include "text.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "uv.hpp"
#include "vec2.hpp"
#include "vertex_vector.hpp"
#include "window.hpp"