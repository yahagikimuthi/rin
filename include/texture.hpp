#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>
#include <utility>
#include "others/type.hpp"

#define STB_IMAGE_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "stb_image.h"
#pragma GCC diagnostic pop

#include "others/error.hpp"
#include "others/util.hpp"
#include "renderer/ebo_manager.hpp"

namespace rin {
class texture final {
  public:
    [[nodiscard]] static auto load_from_file(const std::filesystem::path& path) noexcept
        -> std::expected<texture, error> {
        stbi_set_flip_vertically_on_load(static_cast<int>(true));

        auto width    = 0;
        auto height   = 0;
        auto channels = 0;

        auto* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
        if (data == nullptr) return error::create(error::logic, "Failed to load file pash");

        const auto pixels = std::span<u8>{data, static_cast<std::size_t>(width * height * 4)};

        auto tex = texture{static_cast<u32>(width), static_cast<u32>(height), pixels};

        stbi_image_free(data);
        return tex;
    }

    texture(const texture&) noexcept                    = delete;
    auto operator=(const texture&) noexcept -> texture& = delete;

    texture(texture&& other) noexcept
        : id_{std::exchange(other.id_, 0)}, size_{std::exchange(other.size_, extent{})} {}
    auto operator=(texture&& other) noexcept -> texture& {
        if (this == &other) return *this;

        destroy();

        id_   = std::exchange(other.id_, 0);
        size_ = std::exchange(other.size_, extent{});
        return *this;
    }
    ~texture() noexcept { destroy(); }

    void bind(const u32 unit) const noexcept { glBindTextureUnit(unit, id_); }

  private:
    explicit texture(const u32 width, const u32 height, const std::span<const u8> pixels) noexcept
        : size_{.width = static_cast<f32>(width), .height = static_cast<f32>(height)} {
        glCreateTextures(GL_TEXTURE_2D, 1, &id_);
        glTextureStorage2D(
            id_, 1, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height)
        );
        if (not pixels.empty()) {
            glTextureSubImage2D(
                id_,
                0,
                0,
                0,
                static_cast<GLsizei>(width),
                static_cast<GLsizei>(height),
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                pixels.data()
            );
        }

        glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void destroy() noexcept {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
            id_ = 0;
        }
    }

    GLuint id_{};
    extent size_{};
};
}  // namespace rin