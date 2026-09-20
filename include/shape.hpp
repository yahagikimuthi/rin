#pragma once

#include <concepts>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec2.hpp>

#include "type.hpp"
#include "util.hpp"

namespace rin {
class BaseShape {};

template <typename T>
concept Shape = std::derived_from<T, BaseShape>;

class Quad final : public BaseShape {
  public:
    glm::vec2 position{0, 0};
    glm::vec2 size{0, 0};
    f32       rotation_radius{0.f};
    RGB       color{.r = 1.f, .g = 1.f, .b = 1.f};
};

[[nodiscard]] constexpr auto calc_transform(const Quad& quad) noexcept -> glm::mat4 {
    auto result = glm::translate(glm::mat4(1.f), glm::vec3(quad.position.x, quad.position.y, 0.f));
    result *= glm::scale(glm::mat4(1.f), glm::vec3(quad.size.x, quad.size.y, 1.f));
    return result;
}
}  // namespace rin