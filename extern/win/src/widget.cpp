#include "win/viewport.hpp"
#include "win/widget-constraint.hpp"
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





win::vec2<float> win::widget::draw_string(
    vec2<float>         position,
    std::u32string_view string,
    uint32_t            font_size,
    color               col
) const {
  return viewport().draw_string(position + logical_position(), string, font_size, col);
}





const win::viewport& win::widget::viewport() const {
  if (root_ptr_ == nullptr) {
    throw std::runtime_error{"viewport not set"};
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





void win::widget::render() {
  on_render();
  invalid_ = false;
  for (const auto& [child, constraint]: children_) {
    if (child->invalid_layout_) {
      compute_child_layout(child, constraint);
    }

    child->render();
  }
}






void win::widget::update() {
  on_update();
  for (const auto& [child, _]: children_) {
    child->update();
  }
}





void win::widget::compute_child_layout(widget* child, widget_constraint constraint) {
  auto size_request = constraint.realize_size(realized_size_);

  child->scale_ = scale_;
  child->on_layout(size_request);

  if (std::holds_alternative<dimension_compute_constraint>(constraint.width)) {
    constraint.width = size_request.x.value_or(0.f);
  }
  if (std::holds_alternative<dimension_compute_constraint>(constraint.height)) {
    constraint.height = size_request.y.value_or(0.f);
  }

  auto [x, y, w, h] = constraint.realize(realized_size_);

  child->compute_layout(vec2<float>{x, y} + position_, {w, h}, scale_);
}





void win::widget::compute_layout(vec2<float> position, vec2<float> size, float scale) {
  if (position_ == position && realized_size_ == size && scale_ == scale) {
    return;
  }

  position_       = position;
  realized_size_  = size;
  scale_          = scale;

  invalid_layout_ = false;

  invalidate();

  for (auto [child, constraint]: children_) {
    compute_child_layout(child, constraint);
  }
}





void win::widget::add_child(widget* child, widget_constraint constraint) {
  if (child == nullptr) {
    return;
  }

  if (root_ptr_ != nullptr) {
    child->viewport(viewport());
  }

  children_.emplace_back(child, constraint);

  invalidate();
}





void win::widget::viewport(const win::viewport& vp) {
  root_ptr_ = &vp;
  for (const auto& [child, _]: children_) {
    child->viewport(viewport());
  }
}
