#include "phodispl/progress-circle.hpp"

#include "resources.hpp"

#include <chrono>
#include <cmath>

#include <gl/primitives.hpp>




progress_circle::progress_circle() :
  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_progress_circle_vs_sv(),
    resources::shader_progress_circle_fs_sv()
  },
  shader_progress_        {shader_.uniform("value")},
  shader_alpha_           {shader_.uniform("alpha")},
  shader_trafo_continuous_{shader_.uniform("transformContinuous")},
  shader_trafo_           {shader_.uniform("transform")}
{}





void progress_circle::value(float value) {
  value_ = std::clamp(value, 0.f, 1.f);
}





void progress_circle::on_update() {
  if (visible()) {
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
  if (!visible()) {
    return;
  }

  shader_.use();
  glUniform1f(shader_progress_, value_);
  glUniform1f(shader_alpha_,    opacity());

  float rotation = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count() / 100.f;

  win::set_uniform_mat4(shader_trafo_continuous_, rotation_matrix(rotation));
  win::set_uniform_mat4(shader_trafo_, trafo_mat_logical({0.f, 0.f}, logical_size()));

  quad_.draw();
}
