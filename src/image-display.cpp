#include "logcerr/log.hpp"
#include "phodispl/config.hpp"
#include "phodispl/image-display.hpp"
#include "resources.hpp"

#include <gl/primitives.hpp>





image_display::image_display() :
  crossfade_(
    1.f,
    global_config().animation_view_next_ms.count(),
    global_config().animation_view_next_interpolation
  ),
  quad_{gl::primitives::quad()},
  crossfade_shader_{resources::shader_crossfade_vs(), resources::shader_crossfade_fs()},
  crossfade_shader_factor_a_{crossfade_shader_.uniform("factorA")},
  crossfade_shader_factor_b_{crossfade_shader_.uniform("factorB")}
{
  crossfade_shader_.use();
  glUniform1i(crossfade_shader_.uniform("textureSamplerA"), 0);
  glUniform1i(crossfade_shader_.uniform("textureSamplerB"), 1);
}





void image_display::active(std::shared_ptr<image> next_image) {
  previous_ = std::move(current_);
  current_  = std::move(next_image);

  crossfade_.animate_to(1.f);
}





void image_display::on_update() {
  current_->update();

  if (current_->take_damage()) {
    invalidate();
  }

  if (crossfade_.changed()) {
    invalidate();
  } else {
    previous_.reset();
  }
}





namespace {
  void crossfade_image(image* img, float factor, GLint uni) {
    if (img == nullptr) {
      return;
    }

    if (auto frame = img->current_frame()) {
      frame->texture().bind();
      glUniform4f(uni, factor, factor, factor, factor);
    }
  }
}



void image_display::on_render() {
  crossfade_shader_.use();

  auto factor = crossfade_.factor();

  glActiveTexture(GL_TEXTURE1);
  crossfade_image(previous_.get(), 1.f - factor, crossfade_shader_factor_b_);

  glActiveTexture(GL_TEXTURE0);
  crossfade_image(current_.get(), factor, crossfade_shader_factor_a_);

  quad_.draw();
}
