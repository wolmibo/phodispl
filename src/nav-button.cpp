#include "phodispl/nav-button.hpp"
#include "gl/primitives.hpp"
#include "phodispl/config.hpp"
#include "phodispl/fade-widget.hpp"
#include "resources.hpp"

#include <chrono>
#include <string_view>

#include <win/mouse-button.hpp>
#include <win/types.hpp>
#include <win/viewport.hpp>



nav_button::nav_button(bool left, std::move_only_function<void(void)> on_click) :
  left_{left},

  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_plane_object_vs(),
    resources::shader_nav_button_fs()
  },

  shader_color_back_ {shader_.uniform("colorBack")},
  shader_color_front_{shader_.uniform("colorFront")},
  shader_trafo_      {shader_.uniform("transform")},
  shader_scale_x_    {shader_.uniform("scaleX")},

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





namespace {
  void gray(GLint uni, float value, float alpha) {
    glUniform4f(uni, value * alpha, value * alpha, value * alpha, alpha);
  }
}




void nav_button::on_render() {
  if (!visible()) {
    return;
  }

  shader_.use();

  switch (mouse_state_) {
    case state::normal:
      gray(shader_color_front_, 0.f, opacity() * 0.5f);
      gray(shader_color_back_,  1.f, opacity() * 0.8f);
      break;
    case state::hover:
      gray(shader_color_front_, 0.f, opacity() * 0.7f);
      gray(shader_color_back_,  1.f, opacity() * 0.9f);
      break;
    case state::down:
      gray(shader_color_front_, 0.2f, opacity() * 0.8f);
      gray(shader_color_back_,   1.f, opacity());
      break;
  }

  glUniform1f(shader_scale_x_, left_ ? -1.f : 1.f);

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
