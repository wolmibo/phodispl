#include "win/view.hpp"

#include <algorithm>



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





bool win::view::invalid() const {
  if (invalid_) {
    return true;
  }

  return std::ranges::any_of(children_, [](const auto& pair) {
      return pair.first->invalid();
  });
}





void win::view::render() {
  on_render();
  invalid_ = false;
  for (const auto& [child, _]: children_) {
    child->render();
  }
}





void win::view::compute_layout(vec2<float> position, vec2<float> size, float scale) {
  position_      = position;
  realized_size_ = size;
  scale_         = scale;

  for (const auto& [child, constraint]: children_) {
    auto [x, y, w, h] = constraint.realize(realized_size_);
    child->compute_layout({x, y}, {w, h}, scale);
  }
}





void win::view::add_child(std::shared_ptr<view> child, view_constraint constraint) {
  children_.emplace_back(std::move(child), constraint);

  invalidate();
}
