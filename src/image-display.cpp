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

  shader_{resources::shader_plane_uv_vs(), resources::shader_plane_fs()},
  shader_factor_   {shader_.uniform("factor")},
  shader_transform_{shader_.uniform("transform")},

  exposure_(
    1.f,
    global_config().animation_view_snap_ms.count(),
    global_config().animation_view_snap_curve
  ),
  scale_filter_{global_config().filter}
{}





void image_display::active(std::shared_ptr<image> next_image) {
  previous_ = std::move(current_);
  current_  = std::move(next_image);

  if (previous_.get() != current_.get()) {
    crossfade_.start();
  }
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





void image_display::scale(scale_mode mode) {
  scale_mode_ = mode;
  if (std::holds_alternative<dynamic_scale>(scale_mode_)) {
    position_ = {0.f, 0.f};
  }
  invalidate();
}



void image_display::scale_multiply_at(float factor, win::vec2<float> position) {
  if (auto* value = std::get_if<float>(&scale_mode_)) {
    scale_mode_ = *value * factor;

  } else if (auto* mode = std::get_if<dynamic_scale>(&scale_mode_);
             mode != nullptr && current_) {
    if (auto frame = current_->current_frame()) {
      scale_mode_ = scale_dynamic(*frame, *mode) * factor;
    }
  }

  position = position - 0.5f * logical_size();

  position_ = factor * (position_ - position) + position;

  invalidate();
}





void image_display::translate(win::vec2<float> delta) {
  position_ = position_ + delta;
  invalidate();
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
  [[nodiscard]] std::shared_ptr<frame> current_frame(image* img) {
    if (img == nullptr) {
      return {};
    }
    return img->current_frame();
  }



  void crossfade_image(float factor, float exposure, GLint uni) {
    glUniform4f(uni, factor * exposure, factor * exposure, factor * exposure, factor);
  }



  void set_scale_filter(scale_filter filter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, std::to_underlying(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, std::to_underlying(filter));
  }
}



void image_display::on_render() {
  shader_.use();


  if (auto frame = current_frame(current_.get())) {
    frame->texture().bind();
    set_scale_filter(scale_filter_);

    win::set_uniform_mat4(shader_transform_, matrix_for(*frame));
    crossfade_image(1.f, *exposure_, shader_factor_);
  }

  quad_.draw();
}





float image_display::scale_dynamic(frame& f, dynamic_scale scale) const {
  auto size = logical_size();

  float scale_x = size.x / f.real_width();
  float scale_y = size.y / f.real_height();

  if (scale == dynamic_scale::fit) {
    return std::min(scale_x, scale_y);
  }
  return std::max(scale_x, scale_y);
}





win::mat4 image_display::matrix_for(frame& f) const {
  auto size = logical_size();

  float s = 1.f;
  if (const auto* scale = std::get_if<float>(&scale_mode_)) {
    s = *scale;
  } else if (const auto* mode = std::get_if<dynamic_scale>(&scale_mode_)) {
    s = scale_dynamic(f, *mode);
  }

  float sx = s * f.real_width()  / size.x;
  float sy = s * f.real_height() / size.y;

  float px =  position_.x / size.x / 0.5f;
  float py = -position_.y / size.y / 0.5f;

  return {
     sx, 0.f, 0.f, 0.f,
    0.f,  sy, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
     px,  py, 0.f, 1.f
  };
}
