#include <gl/base.hpp>

#include "win/viewport.hpp"





void win::viewport::resize(vec2<float> size, float scale) {
  glViewport(0, 0, size.x * scale, size.y * scale);
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

  widget::render(*this);
}
