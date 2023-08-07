#include "phodispl/nav-button.hpp"
#include "gl/primitives.hpp"
#include "phodispl/config.hpp"
#include "resources.hpp"
#include "win/types.hpp"
#include <chrono>



nav_button::nav_button() :
  alpha_(
    0.f,
    global_config().animation_view_next_ms.count(),
    global_config().animation_view_next_curve
  ),

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

  if (visible_ && !might_hide_) {
    return;
  }

  alpha_.animate_to(1.f);

  visible_    = true;
  might_hide_ = false;
}





void nav_button::on_update() {
  if (std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - last_movement_).count() > 2) {
    if (visible_ && !might_hide_) {
      alpha_.animate_to(0.f);

      might_hide_ = true;
    }
  }


  if (might_hide_ && *alpha_ < 1e-4f) {
    might_hide_ = false;
    visible_    = false;
    alpha_.set_to(0.f);
    invalidate();
  }

  if (alpha_.changed()) {
    invalidate();
  }
}





void nav_button::on_render() {
  if (!visible_) {
    return;
  }

  shader_.use();
  glUniform4f(shader_color_, *alpha_, 0.f, *alpha_, *alpha_);
  win::set_uniform_mat4(shader_trafo_, trafo_mat_logical({0.f, 0.f}, logical_size()));

  quad_.draw();
}
