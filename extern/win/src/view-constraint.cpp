#include "win/view-constraint.hpp"



namespace {
  [[nodiscard]] float extent(float inner, float outer, float margin) {
    if (inner < 0.f) {
      return outer - margin;
    }
    return inner;
  }



  [[nodiscard]] float shift(float extent, float outer, float start, float end) {
    if (start < 0.f && end < 0.f) {
      return 0;
    }

    if (end < 0.f) {
      return start;
    }

    if (start < 0.f) {
      return outer - extent - end;
    }

    return outer - extent - start - end;
  }
}





std::array<float, 4> win::view_constraint::realize(
    vec2<float> inner_size,
    vec2<float> outer_size
) const {
  std::array<float, 4> bounds{0.f};

  bounds[2] = extent(inner_size.x, outer_size.x, left + right);
  bounds[3] = extent(inner_size.y, outer_size.y, top + bottom);

  bounds[0] = shift(bounds[2], outer_size.x, left, right);
  bounds[1] = shift(bounds[3], outer_size.y, top, bottom);

  return bounds;
}
