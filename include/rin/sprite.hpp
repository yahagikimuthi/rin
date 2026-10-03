#pragma once

#include <cassert>
#include <functional>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

#include "rin/color.hpp"
#include "rin/texture.hpp"
#include "rin/types.hpp"
#include "rin/uv.hpp"
#include "rin/vec2.hpp"
#include "rin/vertex_vector.hpp"

/**
 * @file sprite.hpp
 * @brief 2Dスプライト描画の管理および頂点計算を行うモジュール
 */

namespace rin {
/**
 * @class sprite
 * @brief テクスチャ描画に必要な位置・回転・拡大縮小・カラー・UV情報を保持するクラス
 *
 * @warning 本クラスは内部で `texture` への参照（`std::reference_wrapper`）を保持します。
 * 参照先の `texture` インスタンスが破棄された後にアクセスすると未定義動作となるため、
 * `sprite` の生存期間が `texture` の生存期間を超えないように注意してください。
 */
class sprite final {
  public:
    /**
     * @brief テクスチャを指定して sprite インスタンスを生成します。
     *
     * @param tex 参照するテクスチャ（インスタンスの生存期間に注意してください）
     * @return sprite 生成された sprite インスタンス
     */
    [[nodiscard]] static auto make(const texture& tex) noexcept -> sprite { return sprite{tex}; }

    /**
     * @brief 現在の位置座標を取得します。
     * @return vec2 位置座標
     */
    [[nodiscard]] auto position() const noexcept -> vec2 { return vertices_.position(); }

    /**
     * @brief 現在のスケール（拡大率）を取得します。
     * @return vec2 スケール値
     */
    [[nodiscard]] auto scale() const noexcept -> vec2 { return vertices_.scale(); }

    /**
     * @brief 現在の回転角（ラジアン）を取得します。
     * @return f32 回転角（ラジアン）
     */
    [[nodiscard]] auto rotation() const noexcept -> f32 { return vertices_.rotation(); }

    /**
     * @brief 現在の原点（原点オフセット）を取得します。
     * @return vec2 原点座標
     */
    [[nodiscard]] auto origin() const noexcept -> vec2 { return vertices_.origin(); }

    /**
     * @brief 現在のカラーを取得します。
     * @return rgba カラー情報
     */
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }

    /**
     * @brief スプライトのサイズ(幅・高さ)を取得します。
     * @return extent スプライトサイズ
     */
    [[nodiscard]] auto size() noexcept -> extent {
        const auto tex_size = tex_.get().size();
        const auto scale    = calc_vertices().scale();
        return {.width = tex_size.width * scale.x, .height = tex_size.height * scale.y};
    }

    /**
     * @brief 参照しているテクスチャの参照を取得します。
     * @return const texture& テクスチャの参照
     */
    [[nodiscard]] auto setting_texture() const noexcept -> const texture& { return tex_; }

    /**
     * @brief 位置座標を設定します。
     * @param pos 位置座標
     */
    void position(const vec2 pos) noexcept { vertices_.position(pos); }

    /**
     * @brief 位置座標（X, Y成分）を設定します。
     * @param x X座標
     * @param y Y座標
     */
    void position(const f32 x, const f32 y) noexcept { vertices_.position(x, y); }

    /**
     * @brief スケール（拡大率）を設定します。
     * @param scale スケール値
     */
    void scale(const vec2 scale) noexcept { vertices_.scale(scale); }

    /**
     * @brief スケール（X, Y成分）を設定します。
     * @param x X方向スケール
     * @param y Y方向スケール
     */
    void scale(const f32 x, const f32 y) noexcept { vertices_.scale(x, y); }

    /**
     * @brief 回転角を設定します。
     * @param radian 回転角（ラジアン）
     */
    void rotation(const f32 radian) noexcept { vertices_.rotation(radian); }

    /**
     * @brief 原点（原点オフセット）を設定します。
     * @param origin 原点座標
     */
    void origin(const vec2 origin) noexcept { vertices_.origin(origin); }

    /**
     * @brief 原点（X, Y成分）を設定します。
     * @param x 原点X座標
     * @param y 原点Y座標
     */
    void origin(const f32 x, const f32 y) noexcept { vertices_.origin(x, y); }

    /**
     * @brief 描画カラーを設定します。
     * @param col カラー情報（rgba）
     */
    void color(const rgba& col) noexcept { color(col.r, col.g, col.b, col.a); }

    /**
     * @brief 描画カラー（各チャンネル値）を設定します。
     * @param r 赤成分 (0〜255)
     * @param g 緑成分 (0〜255)
     * @param b 青成分 (0〜255)
     * @param a アルファ成分 (0〜255、デフォルト値: 255)
     */
    void color(const u8 r, const u8 g, const u8 b, const u8 a = 255) noexcept {
        dirty_ = true;
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }

    /**
     * @brief 参照するテクスチャを変更・再設定します。
     *
     * @warning 渡すテクスチャの生存期間がこの sprite インスタンスより長くなるようにしてください。
     * @param tex 設定するテクスチャ参照
     */
    void setting_texture(const texture& tex) noexcept {
        dirty_   = true;
        tex_     = std::cref(tex);
        uv_rect_ = uv_rectangle{
            .x = uv_rect_.x, .y = uv_rect_.y, .width = tex.size().width, .height = tex.size().height
        };
    }

    /**
     * @brief スプライトの最新の頂点情報を計算・取得します。
     *
     * @details 変更フラグ（dirty_）が立っている場合は頂点データを更新してから返します。
     * @return const vertex_vector& 算出された頂点データの参照
     */
    [[nodiscard]] auto calc_vertices() noexcept -> const vertex_vector& {
        if (dirty_) update_vertices();
        dirty_ = false;
        return vertices_;
    }

  private:
    explicit sprite(const texture& tex) noexcept
        : tex_{std::cref(tex)},
          uv_rect_{.x = 0.f, .y = 0.f, .width = tex.size().width, .height = tex.size().height} {}

    void update_vertices() noexcept {
        const auto tex_w = tex_.get().size().width;
        const auto tex_h = tex_.get().size().height;

        const auto u0 = uv_rect_.x / tex_w;
        const auto v0 = uv_rect_.y / tex_h;
        const auto u1 = (uv_rect_.x + uv_rect_.width) / tex_w;
        const auto v1 = (uv_rect_.y + uv_rect_.height) / tex_h;

        const auto w = uv_rect_.width;
        const auto h = uv_rect_.height;

        const auto p0 = vec2{.x = 0.f, .y = 0.f};
        const auto p1 = vec2{.x = 0.f + w, .y = 0.f};
        const auto p2 = vec2{.x = 0.f + w, .y = 0.f + h};
        const auto p3 = vec2{.x = 0.f, .y = 0.f + h};

        vertices_.emplace_back(p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(p1, uv{.u = u1, .v = v0}, color_);
        vertices_.emplace_back(p2, uv{.u = u1, .v = v1}, color_);

        vertices_.emplace_back(p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(p2, uv{.u = u1, .v = v1}, color_);
        vertices_.emplace_back(p3, uv{.u = u0, .v = v1}, color_);
    }

    vertex_vector                         vertices_{primitive_triangles};
    std::reference_wrapper<const texture> tex_;
    uv_rectangle                          uv_rect_{};
    rgba                                  color_{white};
    bool                                  dirty_{true};
};

/**
 * @brief sprite インスタンスを生成するフリーのファクトリ関数
 *
 * @warning 参照する `texture` インスタンスが `sprite` より長く生存している必要があります。
 * @param tex 参照するテクスチャ
 * @return sprite 生成された sprite インスタンス
 */
[[nodiscard]] inline auto make_sprite(const texture& tex) noexcept -> sprite {
    return sprite::make(tex);
}
}  // namespace rin