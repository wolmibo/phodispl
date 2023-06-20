#include <gl/base.hpp>

#include "win/viewport.hpp"





void win::viewport::resize(vec2<float> size, float scale) {
  glViewport(0, 0, size.x * scale, size.y * scale);
  compute_layout({0.f, 0.f}, size, scale);
}





void win::viewport::render() {
  widget::render(*this);
}
