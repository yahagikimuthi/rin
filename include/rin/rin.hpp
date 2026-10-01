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

#include "rin/audio.hpp"
#include "rin/clock.hpp"
#include "rin/error.hpp"
#include "rin/extent.hpp"
#include "rin/font.hpp"
#include "rin/key.hpp"
#include "rin/mouse.hpp"
#include "rin/sprite.hpp"
#include "rin/text.hpp"
#include "rin/texture.hpp"
#include "rin/types.hpp"
#include "rin/uv.hpp"
#include "rin/vec2.hpp"
#include "rin/vertex_vector.hpp"
#include "rin/window.hpp"