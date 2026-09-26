#pragma once

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <optional>

#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
struct aabb_bound final {
    [[nodiscard]] static constexpr auto make(const vec2& center, const extent& size) noexcept
        -> aabb_bound {
        const auto min =
            rin::vec2{.x = center.x - (size.width / 2), .y = center.y - (size.height / 2)};
        const auto max =
            rin::vec2{.x = center.x + (size.width / 2), .y = center.y + (size.height / 2)};
        return aabb_bound{min, max};
    }

    [[nodiscard]] constexpr auto center() const noexcept -> vec2 {
        return vec2{
            .x = (bottom_left.x + upper_right.x) * 0.5f, .y = (bottom_left.y + upper_right.y) * 0.5f
        };
    }

    [[nodiscard]] constexpr auto size() const noexcept -> extent {
        return extent{
            .width = upper_right.x - bottom_left.x, .height = upper_right.y - bottom_left.x
        };
    }

    vec2 bottom_left;
    vec2 upper_right;

  private:
    explicit aabb_bound(const vec2& min, const vec2& max) noexcept
        : bottom_left{min}, upper_right{max} {}
};

[[nodiscard]] inline auto make_aabb_bound(const vec2& position, const extent& size) noexcept
    -> aabb_bound {
    return aabb_bound::make(position, size);
}

struct circle_bound final {
    [[nodiscard]] static constexpr auto make(const vec2& Center, const f32 Radius) noexcept
        -> circle_bound {
        return circle_bound{Center, Radius};
    }

    vec2 center;
    f32  radius;

  private:
    explicit circle_bound(const vec2& Center, const f32 Radius) noexcept
        : center{Center}, radius{Radius} {}
};

struct collision_info final {
    vec2 resolution_vector{};
    f32  penetration{};
};

[[nodiscard]] constexpr auto make_circle_bound(const vec2& center, const f32 radius) noexcept
    -> circle_bound {
    return circle_bound::make(center, radius);
}

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

[[nodiscard]] constexpr auto intersects(const circle_bound& circle, const aabb_bound& aabb) noexcept
    -> std::optional<collision_info> {
    return intersects(aabb, circle);
}

[[nodiscard]] constexpr auto intersects(const aabb_bound& lhs, const aabb_bound& rhs) noexcept
    -> bool {
    return (lhs.bottom_left.x <= rhs.upper_right.x and lhs.upper_right.x >= rhs.bottom_left.x) and
           (lhs.bottom_left.y <= rhs.upper_right.y and lhs.upper_right.y >= rhs.bottom_left.y);
}
}  // namespace rin