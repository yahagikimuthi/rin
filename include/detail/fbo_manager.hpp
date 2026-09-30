#pragma once

#include <expected>
#include <utility>

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
            GL_RGBA,
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

        return fbo_manager{fbo_id, texture_id};
    }

    fbo_manager(const fbo_manager&) noexcept                    = delete;
    auto operator=(const fbo_manager&) noexcept -> fbo_manager& = delete;

    fbo_manager(fbo_manager&& other) noexcept
        : fbo_id_{std::exchange(other.fbo_id_, 0)},
          texture_id_{std::exchange(other.texture_id_, 0)} {}
    auto operator=(fbo_manager&& other) noexcept -> fbo_manager& {
        if (this == &other) return *this;

        destroy();

        fbo_id_     = std::exchange(other.fbo_id_, 0);
        texture_id_ = std::exchange(other.texture_id_, 0);

        return *this;
    }

    ~fbo_manager() noexcept { destroy(); }

    void bind(const extent virtual_size) const noexcept {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);
        glViewport(
            0, 0, static_cast<i32>(virtual_size.width), static_cast<i32>(virtual_size.height)
        );
    }

    void clear() const noexcept {
        constexpr float clear_color[] = {0.f, 0.f, 0.f, 1.f};          // NOLINT
        glClearNamedFramebufferfv(fbo_id_, GL_COLOR, 0, clear_color);  // NOLINT
    }

    void bind_for_bit() const noexcept {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

    void unbind() const noexcept {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

  private:
    explicit fbo_manager(const GLuint fbo_id, const GLuint texture_id) noexcept
        : fbo_id_{fbo_id}, texture_id_{texture_id} {}

    void destroy() noexcept {
        if (texture_id_ != 0) glDeleteTextures(1, &texture_id_);
        if (fbo_id_ != 0) glDeleteFramebuffers(1, &fbo_id_);
    }

    GLuint fbo_id_{};
    GLuint texture_id_{};
};
}  // namespace rin::detail