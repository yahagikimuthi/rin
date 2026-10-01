#pragma once

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <optional>

#include "rin/extent.hpp"
#include "rin/types.hpp"
#include "rin/vec2.hpp"

/**
 * @file collision.hpp
 * @brief 衝突判定
 */

namespace rin {
/**
 * @struct aabb_bound
 * @brief 2D座標空間における軸平行境界ボックス（AABB）
 *
 * @details 最小点（左下）と最大点（右上）の座標を保持します。
 */
struct aabb_bound final {
    /**
     * @brief 中心座標とサイズから AABB インスタンスを生成します。
     *
     * @param center 中心位置の座標
     * @param size 幅と高さ
     * @return aabb_bound 生成された AABB インスタンス
     */
    [[nodiscard]] static constexpr auto make(const vec2& center, const extent& size) noexcept
        -> aabb_bound {
        const auto min =
            rin::vec2{.x = center.x - (size.width / 2.0f), .y = center.y - (size.height / 2.0f)};
        const auto max =
            rin::vec2{.x = center.x + (size.width / 2.0f), .y = center.y + (size.height / 2.0f)};
        return aabb_bound{min, max};
    }

    /**
     * @brief AABB の中心座標を取得します。
     *
     * @return vec2 中心座標
     */
    [[nodiscard]] constexpr auto center() const noexcept -> vec2 {
        return vec2{
            .x = (bottom_left.x + upper_right.x) * 0.5f, .y = (bottom_left.y + upper_right.y) * 0.5f
        };
    }

    /**
     * @brief AABB のサイズ（幅と高さ）を取得します。
     *
     * @return extent 幅と高さ
     */
    [[nodiscard]] constexpr auto size() const noexcept -> extent {
        return extent{
            .width = upper_right.x - bottom_left.x, .height = upper_right.y - bottom_left.y
        };
    }

    /// 最小座標（左下）
    vec2 bottom_left;
    /// 最大座標（右上）
    vec2 upper_right;

  private:
    explicit constexpr aabb_bound(const vec2& min, const vec2& max) noexcept
        : bottom_left{min}, upper_right{max} {}
};

/**
 * @brief AABB インスタンスを生成するフリーのファクトリ関数
 *
 * @param position 中心位置の座標
 * @param size 幅と高さ
 * @return aabb_bound 生成された AABB インスタンス
 */
[[nodiscard]] inline auto make_aabb_bound(const vec2& position, const extent& size) noexcept
    -> aabb_bound {
    return aabb_bound::make(position, size);
}

/**
 * @struct circle_bound
 * @brief 2D座標空間における円形境界（Circle Bound）
 *
 * @details 中心座標と半径を保持します。
 */
struct circle_bound final {
    /**
     * @brief 中心座標と半径から circle_bound インスタンスを生成します。
     *
     * @param Center 中心位置の座標
     * @param Radius 円の半径
     * @return circle_bound 生成された circle_bound インスタンス
     */
    [[nodiscard]] static constexpr auto make(const vec2& Center, const f32 Radius) noexcept
        -> circle_bound {
        return circle_bound{Center, Radius};
    }

    /// 中心座標
    vec2 center;
    /// 半径
    f32 radius;

  private:
    explicit circle_bound(const vec2& Center, const f32 Radius) noexcept
        : center{Center}, radius{Radius} {}
};

/**
 * @brief circle_bound インスタンスを生成するフリーのファクトリ関数
 *
 * @param center 中心位置の座標
 * @param radius 円の半径
 * @return circle_bound 生成された circle_bound インスタンス
 */
[[nodiscard]] constexpr auto make_circle_bound(const vec2& center, const f32 radius) noexcept
    -> circle_bound {
    return circle_bound::make(center, radius);
}

/**
 * @struct collision_info
 * @brief 衝突時の交差情報（押し出しベクトルおよびめり込み量）を保持する構造体
 */
struct collision_info final {
    /// 押し出し方向を表す単位ベクトル
    vec2 resolution_vector{};
    /// めり込み量（深さ）
    f32 penetration{};
};

/**
 * @brief AABB と Circle の衝突（交差）判定を行い、衝突情報を算出します。
 *
 * @param aabb 判定対象の AABB
 * @param circle 判定対象の Circle
 * @return std::optional<collision_info> 衝突している場合は交差情報を返し、非交差時は std::nullopt
 * を返します。
 */
[[nodiscard]] constexpr auto intersects(const aabb_bound& aabb, const circle_bound& circle) noexcept
    -> std::optional<collision_info> {
    const auto closest_x = std::clamp(circle.center.x, aabb.bottom_left.x, aabb.upper_right.x);
    const auto closest_y = std::clamp(circle.center.y, aabb.bottom_left.y, aabb.upper_right.y);

    const auto diff = circle.center - vec2{.x = closest_x, .y = closest_y};
    if (diff.sum_square() >= circle.radius * circle.radius) return std::nullopt;

    const auto dist = diff.abs();
    if (dist > std::numeric_limits<f32>::epsilon()) {
        const auto resolution  = vec2{.x = diff.x / dist, .y = diff.y / dist};
        const auto penetration = circle.radius - dist;
        return collision_info{.resolution_vector = resolution, .penetration = penetration};
    }

    const auto aabb_center  = aabb.center();
    const auto aabb_extents = aabb.size() * 0.5f;
    const auto center_diff  = circle.center - aabb_center;

    const auto overlap_x = aabb_extents.width - std::abs(center_diff.x);
    const auto overlap_y = aabb_extents.height - std::abs(center_diff.y);
    if (overlap_x < overlap_y) {
        const auto sign = (center_diff.x >= 0.f) ? 1.f : -1.f;
        return collision_info{
            .resolution_vector = vec2{.x = sign, .y = 0.f}, .penetration = circle.radius + overlap_x
        };
    }

    const auto sign = (center_diff.y >= 0.f) ? 1.f : -1.f;
    return collision_info{
        .resolution_vector = vec2{.x = 0.f, .y = sign}, .penetration = circle.radius + overlap_y
    };
}

/**
 * @brief Circle と AABB の衝突（交差）判定を行い、衝突情報を算出します。
 *
 * @param circle 判定対象の Circle
 * @param aabb 判定対象の AABB
 * @return std::optional<collision_info> 衝突している場合は交差情報を返し、非交差時は std::nullopt
 * を返します。
 */
[[nodiscard]] constexpr auto intersects(const circle_bound& circle, const aabb_bound& aabb) noexcept
    -> std::optional<collision_info> {
    return intersects(aabb, circle);
}

/**
 * @brief 2つの AABB 同士の交差判定を行います。
 *
 * @param lhs 判定対象の AABB 1
 * @param rhs 判定対象の AABB 2
 * @return true 重なっている場合
 * @return false 重なっていない場合
 */
[[nodiscard]] constexpr auto intersects(const aabb_bound& lhs, const aabb_bound& rhs) noexcept
    -> bool {
    return (lhs.bottom_left.x <= rhs.upper_right.x and lhs.upper_right.x >= rhs.bottom_left.x) and
           (lhs.bottom_left.y <= rhs.upper_right.y and lhs.upper_right.y >= rhs.bottom_left.y);
}
}  // namespace rin