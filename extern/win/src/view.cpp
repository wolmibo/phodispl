#include "win/view.hpp"



win::mat4 win::view::trafo_mat_logical(vec2<float> position, vec2<float> size) const {
  vec2<float> s = vec2_div(logical_size(), size);
  vec2<float> p = 2.f * vec2_div(position + logical_position()
                                 - (size - logical_size()) * 0.5f, size);

  return {
    s.x,   0,   0,   0,
      0, s.y,   0,   0,
      0,   0,   1,   0,
    p.x, p.y,   0,   1
  };
}



win::mat4 win::view::trafo_mat_physical(vec2<float> position, vec2<float> size) const {
  return trafo_mat_logical(position * (1.f / scale()), size * (1.f / scale()));
}





bool win::view::validate() {
  return invalid_.exchange(false);
}
