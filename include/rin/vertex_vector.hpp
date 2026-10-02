#pragma once

#include <bits/ranges_base.h>
#include <bits/stl_iterator_base_types.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <initializer_list>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "rin/color.hpp"
#include "rin/detail/graphics.hpp"
#include "rin/types.hpp"
#include "rin/uv.hpp"
#include "rin/vec2.hpp"

/**
 * @file vertex_vector.hpp
 * @brief 頂点配列クラスの定義
 */

namespace rin {
/**
 * @struct vertex
 * @brief 位置、テクスチャ座標、カラー情報を保持する頂点構造体
 */
struct vertex final {
    constexpr vertex() noexcept = default;

    /**
     * @brief 位置、テクスチャ座標、カラーを指定して頂点を構築します。
     * @param Position 頂点位置座標
     * @param TexCoord テクスチャ座標（UV）
     * @param Color 頂点カラー（RGBA）
     */
    constexpr vertex(const vec2& Position, const uv& TexCoord, const rgba& Color) noexcept
        : position{Position}, tex_coord{TexCoord}, color{Color} {}

    /**
     * @brief 位置とカラーを指定して頂点を構築します（UV座標はデフォルト）。
     * @param Position 頂点位置座標
     * @param Color 頂点カラー（RGBA）
     */
    constexpr vertex(const vec2& Position, const rgba& Color) noexcept
        : position{Position}, color{Color} {}

    /**
     * @brief 位置座標のみを指定して頂点を構築します。
     * @param Position 頂点位置座標
     */
    constexpr vertex(const vec2& Position) noexcept : position{Position} {}

    [[nodiscard]] constexpr auto operator==(const vertex&) const noexcept -> bool = default;

    [[nodiscard]] constexpr auto operator+() const noexcept -> vertex { return *this; }

    /// 頂点位置座標
    vec2 position;

    /// テクスチャ座標（UV）
    uv tex_coord;

    /// 頂点カラー
    rgba color;
};

/**
 * @enum primitive_type
 * @brief 描画するプリミティブ（図形）種別を表す列挙体（基礎型: u8）
 */
enum class primitive_type : u8 {
    /// 点の集合
    points = GL_POINTS,
    /// 線分の集合
    lines = GL_LINES,
    /// 連続した折れ線
    line_strip = GL_LINE_STRIP,
    /// 独立した三角形の集合
    triangles = GL_TRIANGLES,
    /// 連続した三角形（帯状）
    triangle_strip = GL_TRIANGLE_STRIP,
    /// 共通の頂点を中心とする扇状の三角形
    triangle_fan = GL_TRIANGLE_FAN
};

/// 以下はエイリアス
inline constexpr auto primitive_points         = primitive_type::points;
inline constexpr auto primitive_lines          = primitive_type::lines;
inline constexpr auto primitive_line_strip     = primitive_type::line_strip;
inline constexpr auto primitive_triangles      = primitive_type::triangles;
inline constexpr auto primitive_triangle_strip = primitive_type::triangle_strip;
inline constexpr auto primitive_triangle_fan   = primitive_type::triangle_fan;

/**
 * @class vertex_vector
 * @brief 描画プリミティブ用の頂点配列を保持し、位置・回転・スケール等の変換情報を管理するクラス
 */
class vertex_vector final {
  public:
    using reference              = std::vector<vertex>::reference;
    using const_reference        = std::vector<vertex>::const_reference;
    using iterator               = std::vector<vertex>::iterator;
    using const_iterator         = std::vector<vertex>::const_iterator;
    using size_type              = std::vector<vertex>::size_type;
    using difference_type        = std::vector<vertex>::difference_type;
    using allocator_type         = std::vector<vertex>::allocator_type;
    using pointer                = std::vector<vertex>::pointer;
    using const_pointer          = std::vector<vertex>::const_pointer;
    using reverse_iterator       = std::vector<vertex>::reverse_iterator;
    using const_reverse_iterator = std::vector<vertex>::const_reverse_iterator;

    /**
     * @brief 描画プリミティブ種別を指定して vertex_vector インスタンスを構築します。
     * @param type プリミティブ種別
     */
    explicit vertex_vector(const primitive_type type) noexcept : type_{type} {}

    [[nodiscard]] auto operator==(const vertex_vector&) const noexcept -> bool = default;

    /**
     * @brief 現在の位置座標を取得します。
     * @return vec2 位置座標
     */
    [[nodiscard]] auto position() const noexcept -> vec2 { return position_; }

    /**
     * @brief 現在のスケール値を取得します。
     * @return vec2 スケール値
     */
    [[nodiscard]] auto scale() const noexcept -> vec2 { return scale_; }

    /**
     * @brief 現在の回転角（ラジアン）を取得します。
     * @return f32 回転角（ラジアン）
     */
    [[nodiscard]] auto rotation() const noexcept -> f32 { return rotation_; }

    /**
     * @brief 現在の原点オフセットを取得します。
     * @return vec2 原点オフセット
     */
    [[nodiscard]] auto origin() const noexcept -> vec2 { return origin_; }

    /**
     * @brief 現在の設定カラーを取得します。
     * @return rgba カラー情報
     */
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }

    /**
     * @brief 位置座標を設定します。
     * @param pos 位置座標
     * @return vertex_vector& 自身への参照
     */
    auto position(const vec2 pos) noexcept -> vertex_vector& { return position(pos.x, pos.y); }

    /**
     * @brief 位置座標（X, Y成分）を設定します。
     * @param x X座標
     * @param y Y座標
     * @return vertex_vector& 自身への参照
     */
    auto position(const f32 x, const f32 y) noexcept -> vertex_vector& {
        position_ = {.x = x, .y = y};
        return *this;
    }

    /**
     * @brief スケール（拡大率）を設定します。
     * @param scale_vec スケール値
     * @return vertex_vector& 自身への参照
     */
    auto scale(const vec2 scale_vec) noexcept -> vertex_vector& {
        return scale(scale_vec.x, scale_vec.y);
    }

    /**
     * @brief スケール（X, Y成分）を設定します。
     * @param x X方向スケール
     * @param y Y方向スケール
     * @return vertex_vector& 自身への参照
     */
    auto scale(const f32 x, const f32 y) noexcept -> vertex_vector& {
        scale_ = {.x = x, .y = y};
        return *this;
    }

    /**
     * @brief 回転角を設定します。
     * @param radian 回転角（ラジアン）
     * @return vertex_vector& 自身への参照
     */
    auto rotation(const f32 radian) noexcept -> vertex_vector& {
        rotation_ = radian;
        return *this;
    }

    /**
     * @brief 原点オフセットを設定します。
     * @param origin_vec 原点座標
     * @return vertex_vector& 自身への参照
     */
    auto origin(const vec2 origin_vec) noexcept -> vertex_vector& {
        return origin(origin_vec.x, origin_vec.y);
    }

    /**
     * @brief 原点オフセット（X, Y成分）を設定します。
     * @param x 原点X座標
     * @param y 原点Y座標
     * @return vertex_vector& 自身への参照
     */
    auto origin(const f32 x, const f32 y) noexcept -> vertex_vector& {
        origin_ = {.x = x, .y = y};
        return *this;
    }

    /**
     * @brief カラーを設定します。
     * @param col カラー情報（rgba）
     * @return vertex_vector& 自身への参照
     */
    auto color(const rgba& col) noexcept -> vertex_vector& {
        return color(col.r, col.g, col.b, col.a);
    }

    /**
     * @brief カラー（各チャンネル値）を設定します。
     * @param r 赤成分 (0〜255)
     * @param g 緑成分 (0〜255)
     * @param b 青成分 (0〜255)
     * @param a アルファ成分 (0〜255、デフォルト値: 255)
     * @return vertex_vector& 自身への参照
     */
    auto color(const u8 r, const u8 g, const u8 b, const u8 a = 255) noexcept -> vertex_vector& {
        color_ = {.r = r, .g = g, .b = b, .a = a};
        return *this;
    }

    /**
     * @brief 設定された位置・回転・スケール・原点情報からモデル変換行列（glm::mat4）を算出します。
     * @param self 対象の vertex_vector インスタンス
     * @return glm::mat4 計算された変換行列
     */
    [[nodiscard]] friend auto calc_transform_mat(const vertex_vector& self) noexcept -> glm::mat4 {
        auto model =
            glm::translate(glm::mat4(1.f), glm::vec3{self.position_.x, self.position_.y, 0.f});
        if (self.rotation_ != 0.f)
            model = glm::rotate(model, self.rotation_, glm::vec3{0.f, 0.f, 1.f});
        model = glm::scale(model, glm::vec3{self.scale_.x, self.scale_.y, 1.f});
        if (self.origin_.x != 0.f or self.origin_.y != 0.f)
            model = glm::translate(model, glm::vec3{-self.origin_.x, -self.origin_.y, 0.f});

        return model;
    }

    /**
     * @brief 描画プリミティブ種別を取得します。
     * @return primitive_type プリミティブ種別
     */
    [[nodiscard]] auto type() const noexcept -> primitive_type { return type_; }

    /**
     * @brief 内部の頂点配列への読み取り専用スパンを取得します。
     * @return std::span<const vertex> 頂点データへのスパン
     */
    [[nodiscard]] auto span() const noexcept -> std::span<const vertex> { return vec_; }

    [[nodiscard]] auto begin() noexcept -> iterator { return vec_.begin(); }
    [[nodiscard]] auto begin() const noexcept -> const_iterator { return vec_.begin(); }
    [[nodiscard]] auto end() noexcept -> iterator { return vec_.end(); }
    [[nodiscard]] auto end() const noexcept -> const_iterator { return vec_.end(); }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator { return vec_.cbegin(); }
    [[nodiscard]] auto cend() const noexcept -> const_iterator { return vec_.cend(); }
    [[nodiscard]] auto rbegin() noexcept -> reverse_iterator { return vec_.rbegin(); }
    [[nodiscard]] auto rend() noexcept -> reverse_iterator { return vec_.rend(); }
    [[nodiscard]] auto crbegin() const noexcept -> const_reverse_iterator { return vec_.crbegin(); }
    [[nodiscard]] auto crend() const noexcept -> const_reverse_iterator { return vec_.crend(); }

    [[nodiscard]] auto size() const noexcept -> size_type { return vec_.size(); }
    [[nodiscard]] auto max_size() const noexcept -> size_type { return vec_.max_size(); }
    [[nodiscard]] auto capacity() const noexcept -> size_type { return vec_.capacity(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return vec_.empty(); }

    void resize(const size_type n) noexcept { vec_.resize(n); }
    void reserve(const size_type n) noexcept { vec_.reserve(n); }
    void shrink_to_fit() noexcept { vec_.shrink_to_fit(); }

    [[nodiscard]] auto operator[](const size_type i) noexcept -> reference { return vec_[i]; }
    [[nodiscard]] auto operator[](const size_type i) const noexcept -> const_reference {
        return vec_[i];
    }
    [[nodiscard]] auto at(const size_type i) noexcept -> reference { return vec_.at(i); }
    [[nodiscard]] auto at(const size_type i) const noexcept -> const_reference {
        return vec_.at(i);
    }
    [[nodiscard]] auto data() noexcept -> pointer { return vec_.data(); }
    [[nodiscard]] auto data() const noexcept -> const_pointer { return vec_.data(); }
    [[nodiscard]] auto front() noexcept -> reference { return vec_.front(); }
    [[nodiscard]] auto front() const noexcept -> const_reference { return vec_.front(); }
    [[nodiscard]] auto back() noexcept -> reference { return vec_.back(); }
    [[nodiscard]] auto back() const noexcept -> const_reference { return vec_.back(); }

    template <typename InputIterator, typename = std::_RequireInputIter<InputIterator>>
    void assign(InputIterator first, InputIterator last) noexcept {
        vec_.assign(first, last);
    }
    void assign(size_type n, const vertex& v) noexcept { vec_.assign(n, v); }
    void assign(std::initializer_list<vertex> list) noexcept { vec_.assign(list); }

    template <typename R>
        requires requires(std::vector<vertex> vec, R range) { vec.assign_range(range); }
    void assign_range(R&& range) noexcept {
        vec_.assign_range(std::forward<R>(range));
    }

    void push_back(const vertex& v) noexcept { vec_.push_back(v); }

    template <typename... Args>
        requires std::is_constructible_v<vertex, Args...>
    void emplace_back(Args&&... args) noexcept {
        vec_.emplace_back(std::forward<Args>(args)...);
    }

    template <typename R>
        requires requires(std::vector<vertex> vec, R range) { vec.append_range(range); }
    void append_range(R&& range) noexcept {
        vec_.append_range(range);
    }

    void pop_back() noexcept { vec_.pop_back(); }

    auto insert(const iterator& position, const vertex& v) noexcept -> iterator {
        return vec_.insert(position, v);
    }

    auto insert(const iterator& position, size_type n, const vertex& v) noexcept -> iterator {
        return vec_.insert(position, n, v);
    }

    template <typename InputIterator, typename = std::_RequireInputIter<InputIterator>>
    auto insert(const iterator& position, InputIterator first, InputIterator last) noexcept
        -> iterator {
        return vec_.insert(position, first, last);
    }

    auto insert(const iterator& position, std::initializer_list<vertex> v) noexcept -> iterator {
        return vec_.insert(position, v);
    }

    template <typename... Args>
        requires std::is_constructible_v<vertex, Args...>
    auto emplace(const iterator& position, Args&&... args) noexcept -> iterator {
        return vec_.emplace(position, std::forward<Args>(args)...);
    }

    template <typename R>
        requires requires(std::vector<vertex> vec, iterator pos, R range) {
            vec.insert_range(pos, range);
        }
    auto insert_range(const iterator& pos, R&& range) noexcept -> iterator {
        return vec_.insert_range(pos, std::forward<R>(range));
    }
    auto erase(const iterator& position) noexcept -> iterator { return vec_.erase(position); }
    auto erase(const iterator& first, const iterator& last) -> iterator {
        return vec_.erase(first, last);
    }

    template <typename Predicate>
    auto erase_if(Predicate&& pred) noexcept -> size_type {
        return std::erase_if(vec_, std::forward<Predicate>(pred));
    }

    void clear() noexcept { vec_.clear(); }

    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type {
        return vec_.get_allocator();
    }

  private:
    std::vector<vertex> vec_;
    rgba                color_{white};
    vec2                position_{};
    vec2                scale_{.x = 1.f, .y = 1.f};
    vec2                origin_{.x = 0.f, .y = 0.f};
    f32                 rotation_{};
    primitive_type      type_;
};
}  // namespace rin