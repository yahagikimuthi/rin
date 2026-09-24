#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "stb_truetype.h"
#pragma GCC diagnostic pop

#include "others/error.hpp"
#include "others/type.hpp"
#include "texture/texture.hpp"

namespace rin {
class font final {
    struct glyph final {
        uv_rectangle uv_rect{};
        extent       size{};
        vec2         bearing{};
        f32          advance{};
    };

  public:
    [[nodiscard]] static auto create_from_file(
        const std::filesystem::path& path,
        const f32                    font_size,
        const u32                    atlas_width  = 1024,
        const u32                    atlas_height = 1024
    ) noexcept -> std::expected<font, error> {
        auto file = std::ifstream{path, std::ios::binary | std::ios::ate};
        if (not file.is_open()) {
            return std::unexpected(error::create(logic_error, "Failed to open font file."));
        }
        const auto file_size   = file.tellg();
        auto       font_buffer = std::vector<u8>(static_cast<size_t>(file_size));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(font_buffer.data()), file_size);  // NOLINT

        auto atlas_pixels = std::vector<u8>(static_cast<std::size_t>(atlas_width * atlas_height));
        auto baked_chars  = std::vector<stbtt_bakedchar>(96);  // ASCII 32..127 (96文字)

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

        auto f       = font{texture{atlas_width, atlas_height, rgba_pixels.data()}};
        f.font_size_ = font_size;

        for (const auto i : std::views::indices(96u)) {
            const auto& b         = baked_chars[i];
            const auto  codepoint = static_cast<char32_t>(32 + i);

            auto g = glyph{
                .uv_rect =
                    uv_rectangle{
                        .x      = static_cast<f32>(b.x0) / static_cast<f32>(atlas_width),
                        .y      = static_cast<f32>(b.y0) / static_cast<f32>(atlas_height),
                        .width  = static_cast<f32>(b.x1 - b.x0) / static_cast<f32>(atlas_width),
                        .height = static_cast<f32>(b.y1 - b.y0) / static_cast<f32>(atlas_height)
                    },
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

    [[nodiscard]] auto settle_texture() const noexcept -> const texture& { return atlas_texture_; }

    [[nodiscard]] auto font_size() const noexcept -> f32 { return font_size_; }

  private:
    explicit font(texture tex) noexcept : atlas_texture_(std::move(tex)) {}

    std::unordered_map<char32_t, glyph> glyphs_;
    texture                             atlas_texture_;
    f32                                 font_size_{0.f};
};
}  // namespace rin