#include "logcerr/log.hpp"
#include "phodispl/config-types.hpp"
#include "phodispl/config.hpp"
#include "phodispl/image-display.hpp"
#include "resources.hpp"

#include <gl/primitives.hpp>





image_display::image_display() :
  crossfade_(
    global_config().animation_view_next_ms.count(),
    global_config().animation_view_next_curve
  ),
  quad_{gl::primitives::quad()},
  crossfade_shader_{resources::shader_crossfade_vs(), resources::shader_crossfade_fs()},
  crossfade_shader_factor_a_{crossfade_shader_.uniform("factorA")},
  crossfade_shader_factor_b_{crossfade_shader_.uniform("factorB")},

  exposure_(
    1.f,
    global_config().animation_view_snap_ms.count(),
    global_config().animation_view_snap_curve
  ),
  scale_filter_{global_config().filter}
{
  crossfade_shader_.use();
  glUniform1i(crossfade_shader_.uniform("textureSamplerA"), 0);
  glUniform1i(crossfade_shader_.uniform("textureSamplerB"), 1);
}





void image_display::active(std::shared_ptr<image> next_image) {
  previous_ = std::move(current_);
  current_  = std::move(next_image);

  crossfade_.start();
}





void image_display::exposure(float exposure) {
  exposure_.animate_to(exposure);
}



void image_display::exposure_multiply(float diff) {
  exposure_.set_to(*exposure_ * diff);
}





void image_display::scale_filter_toggle() {
  if (scale_filter_ == scale_filter::linear) {
    scale_filter_ = scale_filter::nearest;
  } else {
    scale_filter_ = scale_filter::linear;
  }
  invalidate();
}





void image_display::scale(scale_mode /*unused*/) {

}



void image_display::scale_multiply_at(float /*unused*/, win::vec2<float> /*unused*/) {

}





void image_display::translate(win::vec2<float> /*unused*/) {

}





void image_display::on_update() {
  current_->update();

  if (current_->take_damage()) {
    invalidate();
  }

  if (exposure_.changed()) {
    invalidate();
  }

  if (crossfade_.changed()) {
    invalidate();
  } else {
    previous_.reset();
  }
}





namespace {
  void crossfade_image(image* img, float factor, float exposure, GLint uni) {
    if (img == nullptr) {
      return;
    }

    if (auto frame = img->current_frame()) {
      frame->texture().bind();
      glUniform4f(uni, factor * exposure, factor * exposure, factor * exposure, factor);
    }
  }



  void set_scale_filter(scale_filter filter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, std::to_underlying(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, std::to_underlying(filter));
  }
}



void image_display::on_render() {
  crossfade_shader_.use();

  auto factor = crossfade_.factor();

  glActiveTexture(GL_TEXTURE1);
  crossfade_image(previous_.get(), 1.f - factor, *exposure_, crossfade_shader_factor_b_);
  set_scale_filter(scale_filter_);

  glActiveTexture(GL_TEXTURE0);
  crossfade_image(current_.get(), factor, *exposure_, crossfade_shader_factor_a_);
  set_scale_filter(scale_filter_);

  quad_.draw();
}
