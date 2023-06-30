#include "win/viewport.hpp"
#include "win/widget.hpp"

#include <algorithm>



win::mat4 win::widget::trafo_mat_logical(vec2<float> position, vec2<float> size) const {
  if (root_ptr_ == nullptr) {
    return win::mat4_identity;
  }

  auto vpsize = root_ptr_->logical_size();

  vec2<float> s = vec2_div(size, vpsize);
  vec2<float> p = 2.f * vec2_div(position + logical_position()
                                 - (vpsize - size) * 0.5f, vpsize);

  return {
    s.x, 0.f, 0.f, p.x,
    0.f, s.y, 0.f, p.y,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f
  };
}



win::mat4 win::widget::trafo_mat_physical(vec2<float> position, vec2<float> size) const {
  return trafo_mat_logical(position * (1.f / scale()), size * (1.f / scale()));
}





const win::viewport& win::widget::viewport() const {
  if (root_ptr_ == nullptr) {
    throw std::runtime_error{"viewport only available during rendering"};
  }

  return *root_ptr_;
}





bool win::widget::invalid() const {
  if (invalid_) {
    return true;
  }

  return std::ranges::any_of(children_, [](const auto& pair) {
      return pair.first->invalid();
  });
}





void win::widget::render(const win::viewport& root) {
  root_ptr_ = &root;

  struct clear_root_ptr {
    void operator()(widget* wid) { wid->root_ptr_ = nullptr; }
  };

  std::unique_ptr<widget, clear_root_ptr> root_ptr_guard{this};

  on_render();
  invalid_ = false;
  for (const auto& [child, _]: children_) {
    child->render(root);
  }
}






void win::widget::update() {
  on_update();
  for (const auto& [child, _]: children_) {
    child->update();
  }
}





void win::widget::compute_layout(vec2<float> position, vec2<float> size, float scale) {
  if (position_ == position && realized_size_ == size && scale_ == scale) {
    return;
  }

  position_      = position;
  realized_size_ = size;
  scale_         = scale;

  invalidate();

  for (const auto& [child, constraint]: children_) {
    auto [x, y, w, h] = constraint.realize(realized_size_);
    child->compute_layout(vec2<float>{x, y} + position, {w, h}, scale);
  }
}





void win::widget::add_child(widget* child, widget_constraint constraint) {
  if (child == nullptr) {
    return;
  }

  children_.emplace_back(child, constraint);

  invalidate();
}
