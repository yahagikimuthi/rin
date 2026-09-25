#pragma once

#include <cstddef>
#include <span>
#include <utility>
#include <vector>

#include "glad/glad.h"

#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
struct vertex final {
    vec2 position;
    uv   tex_coord;
    rgba color;
};

enum class primitive_type : u8 {
    points         = GL_POINTS,
    lines          = GL_LINES,
    line_strip     = GL_LINE_STRIP,
    triangles      = GL_TRIANGLES,
    triangle_strip = GL_TRIANGLE_STRIP,
    triangle_fan   = GL_TRIANGLE_FAN
};

inline constexpr auto primitive_points         = primitive_type::points;
inline constexpr auto primitive_lines          = primitive_type::lines;
inline constexpr auto primitive_line_strip     = primitive_type::line_strip;
inline constexpr auto primitive_triangles      = primitive_type::triangles;
inline constexpr auto primitive_triangle_strip = primitive_type::triangle_strip;
inline constexpr auto primitive_triangle_fan   = primitive_type::triangle_fan;

class vertex_vector final {
  public:
    [[nodiscard]] static auto make(const primitive_type type) noexcept -> vertex_vector {
        return vertex_vector{type};
    }

    template <typename... Args>
        requires std::is_constructible_v<vertex, Args...>
    void emplace_back(Args&&... args) noexcept {
        vec_.emplace_back(std::forward<Args>(args)...);
    }

    void resize(const std::size_t n) noexcept { vec_.resize(n); }
    void reserve(const std::size_t n) noexcept { vec_.reserve(n); }
    void clear() noexcept { vec_.clear(); }

    template <typename Self>
    [[nodiscard]] auto operator[](this Self&& self, const std::size_t i) noexcept -> auto&& {
        return std::forward<Self>(self).vec_[i];
    }

    template <typename Self>
    [[nodiscard]] auto at(this Self&& self, const std::size_t i) noexcept -> auto&& {
        return std::forward<Self>(self).vec_.at(i);
    }

    template <typename Self>
    [[nodiscard]] auto begin(this Self&& self) noexcept -> auto {
        return std::forward<Self>(self).vec_.begin();
    }

    template <typename Self>
    [[nodiscard]] auto end(this Self&& self) noexcept -> auto {
        return std::forward<Self>(self).vec_.end();
    }

    template <typename Self>
    [[nodiscard]] auto data(this Self&& self) noexcept -> auto* {
        return std::forward<Self>(self).vec_.data();
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return vec_.size(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return size() == 0; }

    [[nodiscard]] auto type() const noexcept -> primitive_type { return type_; }

    [[nodiscard]] auto get() const noexcept -> std::span<const vertex> { return vec_; }

  private:
    explicit vertex_vector(const primitive_type type) noexcept : type_{type} {}
    std::vector<vertex> vec_;
    primitive_type      type_;
};

[[nodiscard]] inline auto make_vertex_vector(const primitive_type type) noexcept -> vertex_vector {
    return vertex_vector::make(type);
}
}  // namespace rin