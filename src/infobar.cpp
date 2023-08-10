#include "phodispl/infobar.hpp"
#include "phodispl/fade-widget.hpp"
#include "resources.hpp"

#include <gl/primitives.hpp>



infobar::infobar() :
  quad_  {gl::primitives::quad()},
  shader_{
    resources::shader_plane_object_vs(),
    resources::shader_plane_solid_fs()
  },
  shader_trafo_{shader_.uniform("transform")},
  shader_color_{shader_.uniform("color")}
{}





void infobar::on_pointer_enter(win::vec2<float> /*pos*/) {
  mouse_inside_ = true;
}



void infobar::on_pointer_leave() {
  mouse_inside_ = false;
  mouse_leave_ = std::chrono::steady_clock::now();
}





void infobar::show() {
  mouse_leave_ = std::chrono::steady_clock::now();

  fade_widget::show();
}





void infobar::on_update() {
  if (!mouse_inside_ &&
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - mouse_leave_).count() > 2) {
    fade_widget::hide();
  }
}





void infobar::on_render() {
  if (!visible()) {
    return;
  }

  shader_.use();
  glUniform4f(shader_color_, 0.f, 0.f, 0.f, 0.7f * opacity());

  win::set_uniform_mat4(shader_trafo_,
      trafo_mat_logical({0.f, 0.f}, logical_size()));

  quad_.draw();
}
