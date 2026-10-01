#pragma once

#include <array>
#include <expected>
#include <utility>

#include "detail/fullscreen_shader.hpp"
#include "detail/graphics.hpp"

#include "error.hpp"
#include "extent.hpp"
#include "types.hpp"

namespace rin::detail {
class fbo_manager final {
  public:
    [[nodiscard]] static auto try_make(const extent virtual_size) noexcept
        -> std::expected<fbo_manager, error> {
        auto fbo_id     = GLuint{};
        auto texture_id = GLuint{};

        glCreateFramebuffers(1, &fbo_id);

        glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
        glTextureStorage2D(
            texture_id,
            1,
            GL_RGBA8,
            static_cast<i32>(virtual_size.width),
            static_cast<i32>(virtual_size.height)
        );

        glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(fbo_id, GL_COLOR_ATTACHMENT0, texture_id, 0);

        constexpr GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};  // NOLINT
        glNamedFramebufferDrawBuffers(fbo_id, 1, draw_buffers);    // NOLINT

        if (glCheckNamedFramebufferStatus(fbo_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            // 失敗時はリソースを解放
            glDeleteTextures(1, &texture_id);
            glDeleteFramebuffers(1, &fbo_id);
            return make_error(
                runtime_error, "Failed to initialize FBO: Framebuffer is incomplete."
            );
        }

        auto shader = fullscreen_shader::try_make();
        if (not shader)
            return make_error(runtime_error, "Failed to create fullscreen shader.", shader.error());

        return fbo_manager{fbo_id, texture_id, std::move(*shader)};
    }

    fbo_manager(const fbo_manager&) noexcept                    = delete;
    auto operator=(const fbo_manager&) noexcept -> fbo_manager& = delete;

    fbo_manager(fbo_manager&& other) noexcept
        : fbo_id_{std::exchange(other.fbo_id_, 0)},
          texture_id_{std::exchange(other.texture_id_, 0)},
          shader_{std::move(other.shader_)} {}
    auto operator=(fbo_manager&& other) noexcept -> fbo_manager& {
        if (this == &other) return *this;

        destroy();

        fbo_id_     = std::exchange(other.fbo_id_, 0);
        texture_id_ = std::exchange(other.texture_id_, 0);
        shader_     = std::move(other.shader_);

        return *this;
    }

    ~fbo_manager() noexcept { destroy(); }

    void bind(
        const extent virtual_size, const f32 r, const f32 g, const f32 b, const f32 a
    ) const noexcept {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);
        glViewport(
            0, 0, static_cast<i32>(virtual_size.width), static_cast<i32>(virtual_size.height)
        );
        const auto clear_color = std::array<f32, 4>{r, g, b, a};
        glClearNamedFramebufferfv(fbo_id_, GL_COLOR, 0, clear_color.data());  // NOLINT
    }

    void unbind() const noexcept {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void render() const noexcept { shader_.render(texture_id_); }

  private:
    explicit fbo_manager(
        const GLuint fbo_id, const GLuint texture_id, fullscreen_shader shader
    ) noexcept
        : fbo_id_{fbo_id}, texture_id_{texture_id}, shader_{std::move(shader)} {}

    void destroy() noexcept {
        if (texture_id_ != 0) glDeleteTextures(1, &texture_id_);
        if (fbo_id_ != 0) glDeleteFramebuffers(1, &fbo_id_);
    }

    GLuint            fbo_id_{};
    GLuint            texture_id_{};
    fullscreen_shader shader_;
};
}  // namespace rin::detail