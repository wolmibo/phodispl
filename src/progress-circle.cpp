#include "phodispl/progress-circle.hpp"
#include "phodispl/viewport.hpp"

#include "resources.hpp"

#include <algorithm>
#include <cmath>



namespace {
  [[nodiscard]] mat4 rotation_matrix(float rot) {
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



progress_circle::progress_circle(std::shared_ptr<viewport> vp) :
  viewport_{std::move(vp)},

  shader_{
    resources::shader_progress_circle_vs(),
    resources::shader_progress_circle_fs()
  },
  shader_progress_uniform_  {shader_.uniform("value")},
  shader_alpha_uniform_     {shader_.uniform("alpha")},
  shader_continuous_uniform_{shader_.uniform("transform_continuous")},

  start_{std::chrono::high_resolution_clock::now()}
{
  viewport::assert_shader_compat(shader_, "SHADER_PROGRESS_CIRCLE_VS");

  box_.width        = 64.0;
  box_.height       = 64.0;
  box_.anchor       = placement::center;
  box_.align_pixels = false;
}



void progress_circle::render(float progress, float alpha) const {
  progress = std::clamp<float>(progress, 0.0f, 1.0f);
  shader_.use();
  glUniform1f(shader_progress_uniform_, progress);
  glUniform1f(shader_alpha_uniform_,    alpha);

  float rotation = clock().count() / -100.f;

  glUniformMatrix4fv(shader_continuous_uniform_, 1, GL_FALSE,
    rotation_matrix(rotation).data());

  viewport_->render(box_);
}
