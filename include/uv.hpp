#pragma once

#include <compare>

#include "type.hpp"

namespace rin {
struct uv final {
    f32 u{0.f};
    f32 v{0.f};

    auto operator==(const uv&) const noexcept -> bool = default;
};
}  // namespace rin