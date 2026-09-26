#pragma once

#include <algorithm>

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

[[nodiscard]] constexpr auto make_circle_bound(const vec2& center, const f32 radius) noexcept
    -> circle_bound {
    return circle_bound::make(center, radius);
}

[[nodiscard]] constexpr auto intersects(const aabb_bound& aabb, const circle_bound& circle) noexcept
    -> bool {
    const auto closest_x = std::clamp(circle.center.x, aabb.bottom_left.x, aabb.upper_right.x);
    const auto closest_y = std::clamp(circle.center.y, aabb.bottom_left.y, aabb.upper_right.y);

    const auto diff = circle.center - vec2{.x = closest_x, .y = closest_y};
    return diff.sum_square() < circle.radius * circle.radius;
}

[[nodiscard]] constexpr auto intersects(const circle_bound& circle, const aabb_bound& aabb) noexcept
    -> bool {
    return intersects(aabb, circle);
}

[[nodiscard]] constexpr auto intersects(const aabb_bound& lhs, const aabb_bound& rhs) noexcept
    -> bool {
    return (lhs.bottom_left.x <= rhs.upper_right.x and lhs.upper_right.x >= rhs.bottom_left.x) and
           (lhs.bottom_left.y <= rhs.upper_right.y and lhs.upper_right.y >= rhs.bottom_left.y);
}
}  // namespace rin