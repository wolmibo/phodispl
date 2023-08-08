#include "phodispl/nav-button.hpp"
#include "gl/primitives.hpp"
#include "phodispl/config.hpp"
#include "phodispl/fade-widget.hpp"
#include "resources.hpp"
#include "win/mouse-button.hpp"
#include "win/types.hpp"
#include <chrono>



nav_button::nav_button(std::move_only_function<void(void)> on_click) :
  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_plane_object_vs(),
    resources::shader_plane_solid_fs()
  },

  shader_color_{shader_.uniform("color")},
  shader_trafo_{shader_.uniform("transform")},

  on_click_{std::move(on_click)}
{}





void nav_button::show() {
  last_movement_ = std::chrono::steady_clock::now();

  fade_widget::show();
}





void nav_button::on_update() {
  if (mouse_state_ == state::normal &&
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - last_movement_).count() > 2) {
    fade_widget::hide();
  }
}





void nav_button::on_render() {
  if (!visible()) {
    return;
  }

  shader_.use();
  switch (mouse_state_) {
    case state::normal:
      glUniform4f(shader_color_, opacity(), 0.f, opacity(), opacity());
      break;
    case state::hover:
      glUniform4f(shader_color_, opacity(), 0.f, 0.f, opacity());
      break;
    case state::down:
      glUniform4f(shader_color_, 0.f, 0.f, opacity(), opacity());
      break;
  }
  win::set_uniform_mat4(shader_trafo_, trafo_mat_logical({0.f, 0.f}, logical_size()));

  quad_.draw();
}





void nav_button::on_pointer_enter(win::vec2<float> /*pos*/) {
  last_movement_ = std::chrono::steady_clock::now();
  mouse_state_ = state::hover;
  invalidate();
}



void nav_button::on_pointer_leave() {
  last_movement_ = std::chrono::steady_clock::now();
  mouse_state_ = state::normal;
  invalidate();
}



void nav_button::on_pointer_press(win::vec2<float> /*pos*/, win::mouse_button btn) {
  if (btn == win::mouse_button::left) {
    last_movement_ = std::chrono::steady_clock::now();
    mouse_state_ = state::down;
    invalidate();
  }
}



void nav_button::on_pointer_release(win::vec2<float> /*pos*/, win::mouse_button btn) {
  if (btn == win::mouse_button::left && mouse_state_ == state::down) {
    last_movement_ = std::chrono::steady_clock::now();
    mouse_state_ = state::hover;
    if (visible() && on_click_) {
      on_click_();
    }
    invalidate();
  }
}
