#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "others/type.hpp"
#include "others/util.hpp"
#include "texture/font.hpp"

namespace rin {
class text final {
    using str_t = std::variant<std::string, std::string_view>;

  public:
    [[nodiscard]] static auto make(const font& font_ref) noexcept -> text { return text{font_ref}; }

    [[nodiscard]] auto string() const noexcept -> std::string_view {
        return tex_str.visit([](auto&& str) noexcept -> std::string_view { return str; });
    }
    [[nodiscard]] auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }
    [[nodiscard]] auto settle_font() const noexcept -> std::optional<const font&> { return font_; }

    void string(const string_literal auto& str) noexcept { tex_str = std::string_view{str}; }
    void string(const std::string_view str) noexcept { tex_str = std::string{str}; }
    void position(const vec2& pos) noexcept { position_ = pos; }
    void position(const f32 x, const f32 y) noexcept { position_ = {.x = x, .y = y}; }
    void color(const rgba& col) noexcept { color_ = col; }
    void color(const u8 r, const u8 g, const u8 b, const u8 a = 0) noexcept {
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }
    void settle_font(const font& font_obj) noexcept { font_.emplace(font_obj); }

  private:
    explicit text(const font& font_obj) noexcept : font_{font_obj} {}
    str_t                      tex_str;
    rgba                       color_;
    vec2                       position_;
    std::optional<const font&> font_{std::nullopt};
};

[[nodiscard]] inline auto make_text(const font& font_ref) noexcept -> text {
    return text::make(font_ref);
}
}  // namespace rin