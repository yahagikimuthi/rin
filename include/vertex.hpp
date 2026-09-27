#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <span>
#include <utility>
#include <vector>

#include "glad/glad.h"

#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
struct vertex final {
    [[nodiscard]] auto operator==(const vertex&) const noexcept -> bool = default;

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
    using reference              = std::vector<vertex>::reference;
    using const_reference        = std::vector<vertex>::const_reference;
    using iterator               = std::vector<vertex>::iterator;
    using const_iterator         = std::vector<vertex>::const_iterator;
    using size_type              = std::vector<vertex>::size_type;
    using difference_type        = std::vector<vertex>::difference_type;
    using allocator_type         = std::vector<vertex>::allocator_type;
    using pointer                = std::vector<vertex>::pointer;
    using const_pointer          = std::vector<vertex>::const_pointer;
    using reverse_iterator       = std::vector<vertex>::reverse_iterator;
    using const_reverse_iterator = std::vector<vertex>::const_reverse_iterator;

    [[nodiscard]] static auto make(const primitive_type type) noexcept -> vertex_vector {
        return vertex_vector{type};
    }

    [[nodiscard]] auto operator==(const vertex_vector&) const noexcept -> bool = default;

    [[nodiscard]] auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] auto scale() const noexcept -> vec2 { return scale_; }
    [[nodiscard]] auto rotation() const noexcept -> f32 { return rotation_; }
    [[nodiscard]] auto origin() const noexcept -> vec2 { return origin_; }

    auto position(const vec2 pos) noexcept -> vertex_vector& { return position(pos.x, pos.y); }
    auto position(const f32 x, const f32 y) noexcept -> vertex_vector& {
        position_ = {.x = x, .y = y};
        return *this;
    }
    auto scale(const vec2 scale_vec) noexcept -> vertex_vector& {
        return scale(scale_vec.x, scale_vec.y);
    }
    auto scale(const f32 x, const f32 y) noexcept -> vertex_vector& {
        scale_ = {.x = x, .y = y};
        return *this;
    }
    auto rotation(const f32 radian) noexcept -> vertex_vector& {
        rotation_ = radian;
        return *this;
    }
    auto origin(const vec2 origin_vec) noexcept -> vertex_vector& {
        return origin(origin_vec.x, origin_vec.y);
    }
    auto origin(const f32 x, const f32 y) noexcept -> vertex_vector& {
        origin_ = {.x = x, .y = y};
        return *this;
    }

    [[nodiscard]] auto begin() noexcept -> iterator { return vec_.begin(); }
    [[nodiscard]] auto begin() const noexcept -> const_iterator { return vec_.begin(); }
    [[nodiscard]] auto end() noexcept -> iterator { return vec_.end(); }
    [[nodiscard]] auto end() const noexcept -> const_iterator { return vec_.end(); }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator { return vec_.cbegin(); }
    [[nodiscard]] auto cend() const noexcept -> const_iterator { return vec_.cend(); }
    [[nodiscard]] auto rbegin() noexcept -> reverse_iterator { return vec_.rbegin(); }
    [[nodiscard]] auto rend() noexcept -> reverse_iterator { return vec_.rend(); }
    [[nodiscard]] auto crbegin() const noexcept -> const_reverse_iterator { return vec_.crbegin(); }
    [[nodiscard]] auto crend() const noexcept -> const_reverse_iterator { return vec_.crend(); }

    [[nodiscard]] auto size() const noexcept -> size_type { return vec_.size(); }
    [[nodiscard]] auto max_size() const noexcept -> size_type { return vec_.max_size(); }
    [[nodiscard]] auto capacity() const noexcept -> size_type { return vec_.capacity(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return vec_.empty(); }

    void resize(const size_type n) noexcept { vec_.resize(n); }
    void reserve(const size_type n) noexcept { vec_.reserve(n); }
    void shrink_to_fit() noexcept { vec_.shrink_to_fit(); }

    [[nodiscard]] auto operator[](const size_type i) noexcept -> reference { return vec_[i]; }
    [[nodiscard]] auto operator[](const size_type i) const noexcept -> const_reference {
        return vec_[i];
    }
    [[nodiscard]] auto at(const size_type i) noexcept -> reference { return vec_.at(i); }
    [[nodiscard]] auto at(const size_type i) const noexcept -> const_reference {
        return vec_.at(i);
    }
    [[nodiscard]] auto data() noexcept -> pointer { return vec_.data(); }
    [[nodiscard]] auto data() const noexcept -> const_pointer { return vec_.data(); }
    [[nodiscard]] auto front() noexcept -> reference { return vec_.front(); }
    [[nodiscard]] auto front() const noexcept -> const_reference { return vec_.front(); }
    [[nodiscard]] auto back() noexcept -> reference { return vec_.back(); }
    [[nodiscard]] auto back() const noexcept -> const_reference { return vec_.back(); }

    template <typename... Args>
        requires std::is_constructible_v<vertex, Args...>
    void emplace_back(Args&&... args) noexcept {
        vec_.emplace_back(std::forward<Args>(args)...);
    }

    void clear() noexcept { vec_.clear(); }

    [[nodiscard]] auto type() const noexcept -> primitive_type { return type_; }

    [[nodiscard]] auto get() const noexcept -> std::span<const vertex> { return vec_; }

    [[nodiscard]] auto calc_transform_mat() const noexcept -> glm::mat4 {
        auto model = glm::translate(glm::mat4(1.f), glm::vec3{position_.x, position_.y, 0.f});
        if (rotation_ != 0.f) model = glm::rotate(model, rotation_, glm::vec3{0.f, 0.f, 1.f});
        model = glm::scale(model, glm::vec3{scale_.x, scale_.y, 1.f});

        if (origin_.x != 0.f or origin_.y != 0.f)
            model = glm::translate(model, glm::vec3{-origin_.x, -origin_.y, 0.f});

        return model;
    }

  private:
    explicit vertex_vector(const primitive_type type) noexcept : type_{type} {}
    std::vector<vertex> vec_;
    vec2                position_{};
    vec2                scale_{.x = 1.f, .y = 1.f};
    vec2                origin_{.x = 0.f, .y = 0.f};
    f32                 rotation_{};
    primitive_type      type_;
};

[[nodiscard]] inline auto make_vertex_vector(const primitive_type type) noexcept -> vertex_vector {
    return vertex_vector::make(type);
}
}  // namespace rin