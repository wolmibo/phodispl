#include "phodispl/config.hpp"
#include "phodispl/progress-circle.hpp"
#include "resources.hpp"

#include <chrono>
#include <cmath>

#include <gl/primitives.hpp>




progress_circle::progress_circle() :
  alpha_(
    0.f,
    global_config().animation_view_next_ms.count(),
    global_config().animation_view_next_curve
  ),

  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_progress_circle_vs(),
    resources::shader_progress_circle_fs()
  },
  shader_progress_        {shader_.uniform("value")},
  shader_alpha_           {shader_.uniform("alpha")},
  shader_trafo_continuous_{shader_.uniform("transformContinuous")},
  shader_trafo_           {shader_.uniform("transform")}
{}





void progress_circle::value(float value) {
  float new_value = std::clamp(value, 0.f, 1.f);

  if (new_value != value_) {
    value_ = new_value;
    invalidate();
  }
}





void progress_circle::show() {
  if (visible_) {
    return;
  }

  alpha_.animate_to(1.f);

  visible_    = true;
  might_hide_ = false;
}





void progress_circle::hide() {
  if (!visible_ || might_hide_) {
    return;
  }

  alpha_.animate_to(0.f);

  might_hide_ = true;
}





void progress_circle::on_update() {
  if (might_hide_ && *alpha_ < 1e-4f) {
    might_hide_ = false;
    visible_    = false;
    alpha_.set_to(0.f);
    invalidate();
  }

  if (visible_) {
    invalidate();
  }
}





namespace {
  [[nodiscard]] win::mat4 rotation_matrix(float rot) {
    auto a = std::cos(rot);
    auto b = std::sin(rot);
    return {
        a,  -b, 0.f, 0.f,
        b,   a, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    };
  }
}



void progress_circle::on_render() {
  shader_.use();
  glUniform1f(shader_progress_, value_);
  glUniform1f(shader_alpha_,    *alpha_);

  float rotation = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count() / 100.f;

  win::set_uniform_mat4(shader_trafo_continuous_, rotation_matrix(rotation));
  win::set_uniform_mat4(shader_trafo_, trafo_mat_logical({0.f, 0.f}, logical_size()));

  quad_.draw();
}
