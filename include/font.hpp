#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <span>
#include <unordered_map>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "stb_truetype.h"
#pragma GCC diagnostic pop

#include "detail/default_font_data.hpp"
#include "error.hpp"
#include "extent.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "vec2.hpp"

namespace rin::detail {
struct glyph final {
    uv_rectangle uv_rect{};
    extent       size{};
    vec2         bearing{};
    f32          advance{};
};
}  // namespace rin::detail

namespace rin {
inline constexpr auto default_font_size = 48.f;
class font final {
  public:
    [[nodiscard]] static auto try_make(
        const std::filesystem::path& path,
        const u32                    atlas_width  = 1024,
        const u32                    atlas_height = 1024
    ) noexcept -> std::expected<font, error> {
        auto file = std::ifstream{path, std::ios::binary | std::ios::ate};
        if (not file.is_open()) {
            return std::unexpected(make_error(logic_error, "Failed to open font file."));
        }
        const auto               file_size   = file.tellg();
        static thread_local auto font_buffer = std::vector<u8>{};
        font_buffer.resize(static_cast<std::size_t>(file_size));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(font_buffer.data()), file_size);  // NOLINT

        return try_make(font_buffer, atlas_width, atlas_height);
    }

    [[nodiscard]] static auto try_make(
        const std::span<const u8> buffer       = detail::default_font_binary,
        const u32                 atlas_width  = 1024,
        const u32                 atlas_height = 1024
    ) noexcept -> std::expected<font, error> {
        if (buffer.empty()) return make_error(logic_error, "Buffer size is expected positive.");
        auto atlas_pixels = std::vector<u8>(static_cast<std::size_t>(atlas_width * atlas_height));
        auto baked_chars  = std::vector<stbtt_bakedchar>(96);  // ASCII 32..127 (96文字)

        const auto res = stbtt_BakeFontBitmap(
            buffer.data(),
            0,
            default_font_size,
            atlas_pixels.data(),
            static_cast<i32>(atlas_width),
            static_cast<i32>(atlas_height),
            32,
            96,
            baked_chars.data()
        );

        if (res <= 0) return make_error(logic_error, "Font atlas size is too small.");

        auto rgba_pixels =
            std::vector<u8>(static_cast<std::size_t>(atlas_width * atlas_height * 4));
        for (const auto i : std::views::indices(atlas_pixels.size())) {
            const auto alpha         = atlas_pixels[i];
            rgba_pixels[(i * 4) + 0] = 255;
            rgba_pixels[(i * 4) + 1] = 255;
            rgba_pixels[(i * 4) + 2] = 255;
            rgba_pixels[(i * 4) + 3] = alpha;
        }

        auto f = font{make_texture(atlas_width, atlas_height, rgba_pixels.data())};

        for (const auto i : std::views::indices(96u)) {
            const auto& b         = baked_chars[i];
            const auto  codepoint = static_cast<char32_t>(32 + i);

            auto g = detail::glyph{
                .uv_rect =
                    detail::uv_rectangle{
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
        -> std::optional<const detail::glyph&> {
        const auto it = glyphs_.find(codepoint);
        if (it != glyphs_.end()) return it->second;
        return std::nullopt;
    }

    [[nodiscard]] auto setting_texture() const noexcept -> const texture& { return atlas_texture_; }

  private:
    explicit font(texture tex) noexcept : atlas_texture_(std::move(tex)) {}

    std::unordered_map<char32_t, detail::glyph> glyphs_;
    texture                                     atlas_texture_;
};

[[nodiscard]] inline auto try_make_font(
    const std::filesystem::path& path, const u32 atlas_width = 1024, const u32 atlas_height = 1024
) noexcept -> std::expected<font, error> {
    return font::try_make(path, atlas_width, atlas_height);
}

[[nodiscard]] inline auto try_make_font(
    const std::span<const u8> buffer       = detail::default_font_binary,
    const u32                 atlas_width  = 1024,
    const u32                 atlas_height = 1024
) noexcept -> std::expected<font, error> {
    return font::try_make(buffer, atlas_width, atlas_height);
}
}  // namespace rin