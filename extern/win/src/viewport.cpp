#include "win/viewport.hpp"

#include "win/types.hpp"

#include <cmath>

#include <gl/base.hpp>
#include <gl/primitives.hpp>





namespace {
constexpr std::string_view vertex_shader {R"(
#version 450 core

layout (location=0) in vec4 position;
layout (location=0) uniform mat4 transform;

out vec2 uvCoord;

void main() {
  uvCoord = vec2(0.5, -0.5) * position.xy + vec2(0.5, 0.5);
  gl_Position = transform * position;
}
)"};



constexpr std::string_view fragment_shader { R"(
#version 450 core

out vec4 fragColor;

in vec2 uvCoord;
uniform sampler2D textureSampler;

layout (location=1) uniform vec4 color;

void main() {
  fragColor = texture(textureSampler, uvCoord).r * color;
}
)"};
}



win::vec2<float> win::viewport::draw_string(
    vec2<float>         position,
    std::u32string_view string,
    size_t              font_index,
    uint32_t            size,
    color               col
) const {
  if (font_index >= font_cache_.size()) {
    return {0.f, 0.f};
  }

  if (!font_shader_) {
    font_shader_ = gl::program{vertex_shader, fragment_shader};
  }
  font_shader_.use();
  glUniform4f(1, col[0] * col[3], col[1] * col[3], col[2] * col[3], col[3]);

  if (!font_plane_) {
    font_plane_ = gl::primitives::quad();
  }

  size = std::ceil(size * scale());
  position = scale() * position;

  vec2<float> offset{0.f, 0.f};

  for (char32_t c: string) {
    if (c == 10) {
      offset.x = 0.f;
      offset.y += 1.2 * size;
    } else if (c == 32) {
      offset.x += font_cache_[font_index].get(c, size).advance_x();
    } else {
      const auto& g = font_cache_[font_index].get(c, size);

      auto pos = position + offset;
      pos.x = std::floor(pos.x + g.left());
      pos.y = std::floor(pos.y - g.top());

      set_uniform_mat4(0, trafo_mat_physical(pos, {g.width(), g.height()}));
      g.tex().bind();
      font_plane_.draw();

      offset.x += g.advance_x();
    }
  }

  return offset * (1.f / scale());
}





win::vec2<float> win::viewport::measure_string(
    std::u32string_view text,
    size_t              font_index,
    uint32_t            size
) const {
  if (font_index >= font_cache_.size()) {
    return {0.f, 0.f};
  }

  size = std::ceil(size * scale());
  vec2<float> offset{0.f, 0.f};

  for (char32_t c: text) {
    if (c == 10) {
      offset.x = 0.f;
      offset.y += 1.2 * size;
    } else {
      offset.x += font_cache_[font_index].get(c, size).advance_x();
    }
  }

  return offset * (1.f / scale());
}





win::viewport::viewport() {
  widget::viewport(*this);
}





size_t win::viewport::add_font(gl::glyphs glyphs) {
  font_cache_.emplace_back(std::move(glyphs));
  return font_cache_.size() - 1;
}





void win::viewport::resize(vec2<float> size, float scale) {
  glViewport(0, 0, size.x * scale, size.y * scale);

  auto size_request = make_vec2<std::optional<float>>(size.x, size.y);
  on_layout(size_request);

  compute_layout({0.f, 0.f}, size, scale);
}





void win::viewport::render() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(
      bg_color_[0] * bg_color_[3],
      bg_color_[1] * bg_color_[3],
      bg_color_[2] * bg_color_[3],
      bg_color_[3]
  );

  glClear(GL_COLOR_BUFFER_BIT);

  widget::render();
}
