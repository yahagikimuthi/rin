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
}  // namespace rin
namespace rin::quad {
class Quad final : public BaseShape {
  public:
    glm::vec2 position{0, 0};
    glm::vec2 size{0, 0};
    f32       rotation_radian{0.f};
    RGB       color{.r = 2.f, .g = 2.f, .b = 2.f};
};

[[nodiscard]] constexpr auto calc_transform(const Quad& quad) noexcept -> glm::mat4 {
    auto model = glm::mat4(1.f);
    model      = glm::translate(model, glm::vec3(quad.position, 0.f));
    if (quad.rotation_radian != 0.f)
        model = glm::rotate(model, quad.rotation_radian, glm::vec3(0.f, 0.f, 1.f));

    model = glm::scale(model, glm::vec3(quad.size, 1.f));
    return model;
}
}  // namespace rin::quad

namespace rin {
using Quad = quad::Quad;
}