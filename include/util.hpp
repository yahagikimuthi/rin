#pragma once

#include <compare>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>

#include "type.hpp"

namespace rin {
struct rgba final {
    u8 r{0};
    u8 g{0};
    u8 b{0};
    u8 a{255};

    [[nodiscard]] constexpr auto operator+() const noexcept -> rgba { return *this; }

    [[nodiscard]] constexpr auto operator==(const rgba&) const noexcept -> bool = default;

    [[nodiscard]] explicit constexpr operator glm::vec4() const noexcept {
        return glm::vec4{r, g, b, a};
    }
};

inline constexpr auto white   = rgba{.r = 255, .g = 255, .b = 255};
inline constexpr auto silver  = rgba{.r = 192, .g = 192, .b = 192};
inline constexpr auto gray    = rgba{.r = 128, .g = 128, .b = 128};
inline constexpr auto black   = rgba{.r = 0, .g = 0, .b = 0};
inline constexpr auto red     = rgba{.r = 255, .g = 0, .b = 0};
inline constexpr auto maroon  = rgba{.r = 128, .g = 0, .b = 0};
inline constexpr auto yellow  = rgba{.r = 255, .g = 255, .b = 0};
inline constexpr auto olive   = rgba{.r = 128, .g = 128, .b = 0};
inline constexpr auto lime    = rgba{.r = 0, .g = 255, .b = 0};
inline constexpr auto green   = rgba{.r = 0, .g = 128, .b = 0};
inline constexpr auto aqua    = rgba{.r = 0, .g = 255, .b = 255};
inline constexpr auto teal    = rgba{.r = 0, .g = 128, .b = 128};
inline constexpr auto blue    = rgba{.r = 0, .g = 0, .b = 255};
inline constexpr auto navy    = rgba{.r = 0, .g = 0, .b = 128};
inline constexpr auto fuchsia = rgba{.r = 255, .g = 0, .b = 255};
inline constexpr auto purple  = rgba{.r = 128, .g = 0, .b = 128};

struct uv final {
    f32 u{0.f};
    f32 v{0.f};

    auto operator==(const uv&) const noexcept -> bool = default;
};
}  // namespace rin