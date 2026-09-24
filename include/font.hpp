#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "vertex.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "stb_truetype.h"
#pragma GCC diagnostic pop

#include "others/error.hpp"
#include "others/type.hpp"
#include "texture.hpp"

namespace rin {
class font final {
    struct glyph final {
        f32    x{};
        f32    y{};
        f32    width{};
        f32    height{};
        extent size{};
        vec2   bearing{};
        f32    advance{};
    };

  public:
    [[nodiscard]] static auto load_from_file(
        const std::filesystem::path& path,
        const f32                    font_size,
        const u32                    atlas_width,
        const u32                    atlas_height
    ) noexcept -> std::expected<font, error> {
        auto file = std::ifstream{path, std::ios::binary | std::ios::ate};
        if (not file.is_open()) return error::create(logic_error, "Failed to open font file.");

        const auto file_size   = file.tellg();
        auto       font_buffer = std::vector<u8>(static_cast<std::size_t>(file_size));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(font_buffer.data()), file_size);  // NOLINT

        auto atlas_pixels = std::vector<u8>(static_cast<std::size_t>(atlas_width * atlas_height));
        auto baked_chars  = std::vector<stbtt_bakedchar>(96);

        const auto res = stbtt_BakeFontBitmap(
            font_buffer.data(),
            0,
            font_size,
            atlas_pixels.data(),
            static_cast<i32>(atlas_width),
            static_cast<i32>(atlas_height),
            32,
            96,
            baked_chars.data()
        );

        if (res <= 0) return error::create(logic_error, "Font atlas size is too small.");

        auto rgba_pixels =
            std::vector<u8>(static_cast<std::size_t>(atlas_width * atlas_height * 4));
        for (const auto i : std::views::indices(atlas_pixels.size())) {
            const auto alpha         = atlas_pixels[i];
            rgba_pixels[(i * 4) + 0] = 255;
            rgba_pixels[(i * 4) + 1] = 255;
            rgba_pixels[(i * 4) + 2] = 255;
            rgba_pixels[(i * 4) + 3] = alpha;
        }

        auto tex     = texture{atlas_width, atlas_height, rgba_pixels.data()};
        auto f       = font{std::move(tex)};
        f.font_size_ = font_size;

        for (const auto i : std::views::indices(96uz)) {
            const auto& b         = baked_chars[i];
            const auto  codepoint = static_cast<char32_t>(32 + i);

            auto g = glyph{
                .x      = static_cast<f32>(b.x0) / static_cast<f32>(atlas_width),
                .y      = static_cast<f32>(b.y0) / static_cast<f32>(atlas_height),
                .width  = static_cast<f32>(b.x1 - b.x0) / static_cast<f32>(atlas_width),
                .height = static_cast<f32>(b.y1 - b.y0) / static_cast<f32>(atlas_height),
                .size =
                    extent{
                        .width  = static_cast<f32>(b.x1 - b.x0),
                        .height = static_cast<f32>(b.y1 - b.y0)
                    },
                .bearing = vec2{.x = b.xoff, .y = b.yoff},
                .advance = b.xadvance
            };
            f.glyphs_[codepoint] = g;
        }
        return f;
    }

    [[nodiscard]] auto glyph_of_point(const char32_t codepoint) const noexcept
        -> std::optional<const glyph&> {
        const auto it = glyphs_.find(codepoint);
        if (it != glyphs_.end()) return it->second;
        return std::nullopt;
    }

    [[nodiscard]] auto font_size() const noexcept -> f32 { return font_size_; }

  private:
    explicit font(texture tex) noexcept : atlas_texture_(std::move(tex)) {}

    std::unordered_map<char32_t, glyph> glyphs_;
    texture                             atlas_texture_;
    f32                                 font_size_{0.f};
};

[[nodiscard]] inline auto calc_text_vertices(
    const font& font_obj, std::string_view text_str, const vec2& position, const rgba& color
) noexcept -> vertex_vector {
    auto vec      = vertex_vector{primitive_triangles};
    auto cursor_x = position.x;
    auto cursor_y = position.y;

    for (const auto c : text_str) {
        if (c == '\n') {
            cursor_x = position.x;
            cursor_y += font_obj.font_size();
            continue;
        }

        const auto g = font_obj.glyph_of_point(static_cast<char32_t>(c));
        if (g == std::nullopt) continue;

        const auto x0 = cursor_x + g->bearing.x;
        const auto y0 = cursor_y + g->bearing.y;
        const auto x1 = x0 + g->size.width;
        const auto y1 = y0 + g->size.height;

        const auto u0 = g->x;
        const auto v0 = g->y;
        const auto u1 = g->x + g->width;
        const auto v1 = g->y + g->height;

        vec.emplace_back(vec2{.x = x0, .y = y0}, uv{.u = u0, .v = v0}, color);
        vec.emplace_back(vec2{.x = x1, .y = y0}, uv{.u = u1, .v = v0}, color);
        vec.emplace_back(vec2{.x = x1, .y = y1}, uv{.u = u1, .v = v1}, color);

        vec.emplace_back(vec2{.x = x0, .y = y0}, uv{.u = u0, .v = v0}, color);
        vec.emplace_back(vec2{.x = x1, .y = y1}, uv{.u = u1, .v = v1}, color);
        vec.emplace_back(vec2{.x = x0, .y = y1}, uv{.u = u0, .v = v1}, color);

        cursor_x += g->advance;
    }

    return vec;
}
}  // namespace rin