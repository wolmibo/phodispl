#include "phodispl/config-types.hpp"
#include "phodispl/config.hpp"
#include "phodispl/image-display.hpp"
#include "pixglot/exception.hpp"
#include "pixglot/square-isometry.hpp"
#include "resources.hpp"
#include "win/widget-constraint.hpp"

#include <gl/primitives.hpp>





image_display::image_display() :
  crossfade_(
    global_config().animation_view_next_ms.count(),
    global_config().animation_view_next_curve
  ),
  quad_{gl::primitives::quad()},

  shader_{resources::shader_crossfade_vs(), resources::shader_crossfade_fs()},
  shader_factor_a_   {shader_.uniform("factorA")},
  shader_factor_b_   {shader_.uniform("factorB")},
  shader_transform_a_{shader_.uniform("transformA")},
  shader_transform_b_{shader_.uniform("transformB")},

  exposure_(
    1.f,
    global_config().animation_view_snap_ms.count(),
    global_config().animation_view_snap_curve
  ),

  scale_filter_{global_config().filter},

  position_(
    {0.f, 0.f},
    global_config().animation_view_snap_ms.count(),
    global_config().animation_view_snap_curve
  )
{
  add_child(&message_box_, win::widget_constraint {
      .width  = 512.f,
      .height = 192.f,
      .margin = win::margin_constraint{
        .start  = 0.f,
        .end    = 0.f,
        .top    = 0.f,
        .bottom = 0.f
      }
  });

  add_child(&progress_circle_, win::widget_constraint {
      .width  = 64.f,
      .height = 64.f,
      .margin = win::margin_constraint{
        .start  = 0.f,
        .end    = 0.f,
        .top    = 0.f,
        .bottom = 0.f
      }
  });

  shader_.use();
  glUniform1i(shader_.uniform("textureSamplerA"), 0);
  glUniform1i(shader_.uniform("textureSamplerB"), 1);
}





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
  scale_mode_ = current_scale(scale_mode_);

  if (std::holds_alternative<dynamic_scale>(mode)) {
    position_.animate_to({0.f, 0.f});
  } else if (auto* scale = std::get_if<float>(&mode)) {
    position_.animate_to((*scale / std::get<float>(scale_mode_)) * *position_);
  }

  scale_mode_target_ = mode;
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

  scale_mode_target_ = scale_mode_;

  position = position - 0.5f * logical_size();

  position_.set_to(factor * (*position_ - position) + position);

  invalidate();
}





void image_display::translate(win::vec2<float> delta) {
  position_.set_to(*position_ + delta);
  invalidate();
}





void image_display::on_update() {
  if (current_) {
    current_->update();

    if (current_->take_damage()) {
      invalidate();
    }

    if (const auto* error = current_->error(); error != nullptr) {
      set_error(error);
    } else {
      message_box_.hide();
      set_error(nullptr);
    }
  } else {
    message_box_.show_message(
        "[204]  No Content",

        "You did not start PhoDispl with any images to load.\n"
        "\n"
        "Make sure to pass an image file or a directory containing image files as "
        "argument to PhoDispl."
    );
  }

  if (current_ && current_->loading()) {
    progress_circle_.show();
    progress_circle_.value(current_->progress());
  } else {
    progress_circle_.hide();
  }

  if (exposure_.changed()) {
    invalidate();
  }

  if (position_.changed()) {
    invalidate();
  } else {
    scale_mode_ = scale_mode_target_;
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



  constexpr win::mat4 out_of_range_matrix {
    1.f, 0.f, 0.f, -10.f,
    0.f, 1.f, 0.f, -10.f,
    0.f, 0.f, 1.f,   0.f,
    0.f, 0.f, 0.f,   1.f,
  };
}



void image_display::on_render() {
  shader_.use();

  float factor = *crossfade_;

  if (auto frame = current_frame(current_.get())) {
    glActiveTexture(GL_TEXTURE0);
    frame->texture().bind();
    set_scale_filter(scale_filter_);

    win::set_uniform_mat4(shader_transform_a_, matrix_for(*frame));
    crossfade_image(factor, *exposure_, shader_factor_a_);
  } else {
    win::set_uniform_mat4(shader_transform_a_, out_of_range_matrix);
  }

  if (auto frame = current_frame(previous_.get())) {
    glActiveTexture(GL_TEXTURE1);
    frame->texture().bind();
    set_scale_filter(scale_filter_);

    win::set_uniform_mat4(shader_transform_b_, matrix_for(*frame));
    crossfade_image(1.f - factor, *exposure_, shader_factor_b_);

    glActiveTexture(GL_TEXTURE0);
  } else {
    win::set_uniform_mat4(shader_transform_b_, out_of_range_matrix);
  }

  quad_.draw();
}





float image_display::scale_any(frame& f, scale_mode mode) const {
  if (auto* dynamic = std::get_if<dynamic_scale>(&mode)) {
    return scale_dynamic(f, *dynamic);
  }

  if (auto* value = std::get_if<float>(&mode)) {
    return *value;
  }

  return 1.f;
}





float image_display::current_scale(scale_mode mode) const {
  if (auto* value = std::get_if<float>(&mode)) {
    return *value;
  }

  if (auto frame = current_frame(current_.get())) {
    return scale_any(*frame, mode);
  }

  return 1.f;
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





namespace {
  [[nodiscard]] win::mat4 matrix_from(
      float                    sx,
      float                    sy,
      float                    px,
      float                    py,
      pixglot::square_isometry trafo
  ) {
    using enum pixglot::square_isometry;

    win::mat4 matrix {
      0.f, 0.f, 0.f,  px,
      0.f, 0.f, 0.f,  py,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f
    };

    switch (trafo) {
      default:
      case identity:       matrix[0] =  sx; matrix[5] =  sy; break;
      case flip_x:         matrix[0] = -sx; matrix[5] =  sy; break;
      case rotate_ccw:     matrix[4] = -sy; matrix[1] =  sx; break;
      case transpose:      matrix[4] = -sy; matrix[1] = -sx; break;
      case rotate_half:    matrix[0] = -sx; matrix[5] = -sy; break;
      case flip_y:         matrix[0] =  sx; matrix[5] = -sy; break;
      case rotate_cw:      matrix[4] =  sy; matrix[1] = -sx; break;
      case anti_transpose: matrix[1] =  sy; matrix[4] =  sx; break;
    }
    return matrix;
  }
}





win::mat4 image_display::matrix_for(frame& f) const {
  auto size = logical_size();

  float s_source = scale_any(f, scale_mode_);
  float s_target = scale_any(f, scale_mode_target_);

  float factor = position_.clock().factor();

  float s = (1.f - factor) * s_source + factor * s_target;

  float sx = s * f.real_width()  / size.x;
  float sy = s * f.real_height() / size.y;

  auto pos = win::vec2_mul(win::vec2_div(*position_, size), {2.f, -2.f});

  return matrix_from(sx, sy, pos.x, pos.y, f.orientation());
}





namespace {
  [[nodiscard]] std::string bump_first(std::string_view in) {
    std::string out{in};
    if (!out.empty()) {
      out[0] = toupper(out[0]);
    }
    return out;
  }
}



void image_display::set_error(const pixglot::base_exception* error) {
  if (active_error_ == error) {
    return;
  }

  active_error_ = error;

  if (error == nullptr) {
    return;
  }

  if (dynamic_cast<const pixglot::no_stream_access*>(error) != nullptr) {
    message_box_.show_message(
        "[404]  Data Not Found",
        "Cannot access input data."
    );
  } else if (dynamic_cast<const pixglot::no_decoder*>(error) != nullptr) {
    message_box_.show_message(
        "[415]  Unsupported Media Type",
        "None of the decoders recognized this file type."
    );
  } else if (const auto* err = dynamic_cast<const pixglot::decode_error*>(error)) {
    message_box_.show_message(
        "[409]  Failed to Decode",
        "The " + pixglot::to_string(err->decoder()) +
        " decoder reports:\n" +
        bump_first(err->plain())
    );
  } else {
    message_box_.show_message(
        "[500]  Internal Error",
        std::string{error->message()}
    );
  }
}
