#include "phodispl/nav-button.hpp"
#include "gl/primitives.hpp"
#include "phodispl/config.hpp"
#include "phodispl/fade-widget.hpp"
#include "resources.hpp"
#include "win/types.hpp"
#include <chrono>



nav_button::nav_button() :
  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_plane_object_vs(),
    resources::shader_plane_solid_fs()
  },

  shader_color_{shader_.uniform("color")},
  shader_trafo_{shader_.uniform("transform")}
{}





void nav_button::show() {
  last_movement_ = std::chrono::steady_clock::now();

  fade_widget::show();
}





void nav_button::on_update_fw() {
  if (std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - last_movement_).count() > 2) {
    fade_widget::hide();
  }
}





void nav_button::on_render() {
  if (!visible()) {
    return;
  }

  shader_.use();
  glUniform4f(shader_color_, opacity(), 0.f, opacity(), opacity());
  win::set_uniform_mat4(shader_trafo_, trafo_mat_logical({0.f, 0.f}, logical_size()));

  quad_.draw();
}
