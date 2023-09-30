#include "phodispl/nav-button.hpp"
#include "phodispl/config.hpp"
#include "phodispl/fade-widget.hpp"
#include "resources.hpp"

#include <chrono>

#include <gl/primitives.hpp>

#include <win/mouse-button.hpp>
#include <win/types.hpp>
#include <win/viewport.hpp>



nav_button::nav_button(bool left, std::move_only_function<void(void)> on_click) :
  left_{left},

  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_nav_button_vs_sv(),
    resources::shader_nav_button_fs_sv()
  },

  shader_color_back_ {shader_.uniform("colorBack")},
  shader_color_front_{shader_.uniform("colorFront")},
  shader_trafo_      {shader_.uniform("transform")},
  shader_scale_x_    {shader_.uniform("scaleX")},
  shader_scale_r_    {shader_.uniform("scaleR")},
  shader_aa_size_    {shader_.uniform("aaSize")},

  highlight_(
      0.f,
      global_config().animation_ui_highlight_ms.count(),
      global_config().animation_ui_highlight_curve
  ),

  on_click_{std::move(on_click)}
{}





void nav_button::show() {
  last_movement_ = std::chrono::steady_clock::now();

  fade_widget::show();
}





void nav_button::on_update() {
  if (highlight_.changed()) {
    invalidate();
  }

  if (!mouse_down_ &&
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - last_movement_).count() > 2) {
    fade_widget::hide();
  }
}





namespace {
  void gray(GLint uni, float value, float alpha) {
    glUniform4f(uni, value * alpha, value * alpha, value * alpha, alpha);
  }



  [[nodiscard]] float inv_scale(float value) {
    return 1.1f - (value * 0.1f);
  }
}




void nav_button::on_render() {
  if (!visible()) {
    return;
  }

  shader_.use();

  if (mouse_down_) {
    gray(shader_color_front_, 0.2f, opacity() * 0.8f);
    gray(shader_color_back_,   1.f, opacity());

  } else {
    gray(shader_color_front_, 0.f, opacity() * (0.5f + *highlight_ * 0.2f));
    gray(shader_color_back_,  1.f, opacity() * (0.8f + *highlight_ * 0.1f));
  }


  glUniform1f(shader_scale_x_, left_ ? -1.f : 1.f);
  glUniform1f(shader_scale_r_, inv_scale(*highlight_));

  glUniform1f(shader_aa_size_, 64.f / logical_size().x);

  win::set_uniform_mat4(shader_trafo_, trafo_mat_logical({0.f, 0.f}, logical_size()));
  quad_.draw();
}





void nav_button::on_pointer_enter(win::vec2<float> /*pos*/) {
  last_movement_ = std::chrono::steady_clock::now();
  highlight_.animate_to(1.f);

  invalidate();
}



void nav_button::on_pointer_leave() {
  last_movement_ = std::chrono::steady_clock::now();
  highlight_.animate_to(0.f);

  invalidate();
}



void nav_button::on_pointer_press(win::vec2<float> /*pos*/, win::mouse_button btn) {
  if (btn == win::mouse_button::left) {
    last_movement_ = std::chrono::steady_clock::now();
    mouse_down_ = true;

    invalidate();
  }
}



void nav_button::on_pointer_release(win::vec2<float> /*pos*/, win::mouse_button btn) {
  if (btn == win::mouse_button::left && mouse_down_) {
    last_movement_ = std::chrono::steady_clock::now();
    mouse_down_ = false;
    if (visible() && on_click_) {
      on_click_();
    }

    invalidate();
  }
}





bool nav_button::stencil(win::vec2<float> pos) const {
  auto center = logical_position() + 0.5f * logical_size();
  auto diff   = win::vec2_div((pos - center) * inv_scale(*highlight_),
                              0.5f * logical_size());

  return diff.x * diff.x + diff.y * diff.y <= 1.f;
}





void nav_button::on_layout(win::vec2<std::optional<float>>& size) {
  auto ls = viewport().logical_size();

  float s = std::clamp(std::min(ls.x - 6.f * 24.f, ls.y - 3.f * 24.f), 24.f, 64.f);

  size = {s, s};
}
