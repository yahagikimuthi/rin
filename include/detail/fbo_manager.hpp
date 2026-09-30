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
    [[nodiscard]] static auto try_make() noexcept -> std::expected<fbo_manager, error> {
        auto fbo_id = GLuint{};

        glGenFramebuffers(1, &fbo_id);

        // FBO の完全性チェック
        if (glCheckNamedFramebufferStatus(fbo_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return make_error(runtime_error, "Failed to initialize FBO.");

        return fbo_manager{fbo_id};
    }

    fbo_manager(const fbo_manager&) noexcept                    = delete;
    auto operator=(const fbo_manager&) noexcept -> fbo_manager& = delete;

    fbo_manager(fbo_manager&& other) noexcept : fbo_id_{std::exchange(other.fbo_id_, 0)} {}
    auto operator=(fbo_manager&& other) noexcept -> fbo_manager& {
        if (this == &other) return *this;

        destroy();

        fbo_id_ = std::exchange(other.fbo_id_, 0);

        return *this;
    }

    ~fbo_manager() noexcept { destroy(); }

    void bind(const extent& virtual_size) const noexcept {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);
        glViewport(
            0, 0, static_cast<i32>(virtual_size.width), static_cast<i32>(virtual_size.height)
        );
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
    explicit fbo_manager(const GLuint fbo_id) noexcept : fbo_id_{fbo_id} {}

    void destroy() noexcept {
        if (fbo_id_ != 0) glDeleteFramebuffers(1, &fbo_id_);
    }

    GLuint fbo_id_{};
};
}  // namespace rin::detail