#pragma once

#include <compare>

#include "types.hpp"

namespace rin {
struct uv final {
    f32 u{};
    f32 v{};

    auto operator==(const uv&) const noexcept -> bool = default;
};
}  // namespace rin