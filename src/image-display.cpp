#include "phodispl/config-types.hpp"
#include "phodispl/config.hpp"
#include "phodispl/formatting.hpp"
#include "phodispl/image-display.hpp"
#include "resources.hpp"

#include <gl/primitives.hpp>

#include <pixglot/exception.hpp>
#include <pixglot/square-isometry.hpp>

#include <win/widget-constraint.hpp>





image_display::image_display() :
  crossfade_(
    global_config().animation_view_next_ms.count(),
    global_config().animation_view_next_curve
  ),
  quad_{gl::primitives::quad()},

  shader_{resources::shader_crossfade_vs_sv(), resources::shader_crossfade_fs_sv()},
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
      .height = win::dimension_compute_constraint{},
      .margin = win::margin_constraint{
        .start  = 0.f,
        .end    = 0.f,
        .top    = 0.f,
        .bottom = global_config().theme_text_size,
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

  add_child(&infobar_, win::widget_constraint{
      .width  = win::dimension_fill_constraint{},
      .height = win::dimension_compute_constraint{},
      .margin = win::margin_constraint{
        .start  = 0.f,
        .end    = 0.f,
        .top    = 0.f,
        .bottom = {}
      }
  });

  shader_.use();
  glUniform1i(shader_.uniform("textureSamplerA"), 0);
  glUniform1i(shader_.uniform("textureSamplerB"), 1);
}





void image_display::active(std::shared_ptr<image> next_image) {
  previous_ = std::move(current_);
  current_  = std::move(next_image);

  if (previous_.get() == current_.get()) {
    return;
  }

  crossfade_.start();

  if (current_) {
    infobar_.set_image(*current_);
    current_frame_ = current_->current_frame();
    if (current_frame_) {
      infobar_.set_frame(*current_frame_);
    } else {
      infobar_.clear_frame();
    }
  } else {
    infobar_.clear_image();
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

      current_frame_ = current_->current_frame();

      infobar_.set_image(*current_);

      if (current_frame_) {
        infobar_.set_frame(*current_frame_);
      } else {
        infobar_.clear_frame();
      }
    }

    if (const auto* error = current_->error(); error != nullptr) {
      set_error(error, current_->path());
    } else {
      message_box_.hide();
      set_error(nullptr, current_->path());
    }
  } else {
    message_box_.message(
        "[204]  No Content",

        "You did not start PhoDispl with any images to load.\n"
        "\n"
        "Make sure to pass an image file or a directory containing image files as "
        "argument to PhoDispl."
    );
    message_box_.show();
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
  [[nodiscard]] std::optional<pixglot::frame_view> current_frame(image* img) {
    if (img == nullptr) {
      return {};
    }

    if (!global_config().il_show_loading && img->loading()) {
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

  if (current_frame_) {
    glActiveTexture(GL_TEXTURE0);
    current_frame_->texture().bind();
    set_scale_filter(scale_filter_);

    win::set_uniform_mat4(shader_transform_a_, matrix_for(*current_frame_));
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





float image_display::scale_any(const pixglot::frame_view& f, scale_mode mode) const {
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





namespace {
  [[nodiscard]] win::vec2<float> real_size(const pixglot::frame_view& f) {
    auto size = win::make_vec2<float>(f.width(), f.height());

    if (pixglot::flips_xy(f.orientation())) {
      std::swap(size.x, size.y);
    }

    return size;
  }
}





float image_display::scale_dynamic(
    const pixglot::frame_view& f,
    dynamic_scale              scale_mode
) const {
  auto scale = win::vec2_div(logical_size(), real_size(f));

  if (scale_mode == dynamic_scale::fit) {
    return std::min(scale.x, scale.y);
  }
  return std::max(scale.x, scale.y);
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





win::mat4 image_display::matrix_for(const pixglot::frame_view& f) const {
  auto size = logical_size();

  float s_source = scale_any(f, scale_mode_);
  float s_target = scale_any(f, scale_mode_target_);

  float factor = position_.clock().factor();

  float s = (1.f - factor) * s_source + factor * s_target;

  auto scale = s * win::vec2_div(real_size(f), size);

  auto pos = win::vec2_mul(win::vec2_div(*position_, size), {2.f, -2.f});

  return matrix_from(scale.x, scale.y, pos.x, pos.y, f.orientation());
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



void image_display::set_error(
    const pixglot::base_exception* error,
    const std::filesystem::path&   file
) {
  if (active_error_ == error) {
    return;
  }

  active_error_ = error;

  if (error == nullptr) {
    return;
  }

  if (dynamic_cast<const pixglot::no_stream_access*>(error) != nullptr) {
    message_box_.message(
        "[404]  Data Not Found",
        "Cannot access input data.",
        nice_path(file)
    );
  } else if (dynamic_cast<const pixglot::no_decoder*>(error) != nullptr) {
    message_box_.message(
        "[415]  Unsupported Media Type",
        "None of the decoders recognized this file type.",
        nice_path(file)
    );
  } else if (const auto* err = dynamic_cast<const pixglot::decode_error*>(error)) {
    message_box_.message(
        "[409]  Failed to Decode",
        "The " + pixglot::to_string(err->decoder()) +
        " decoder reports:\n" +
        bump_first(err->plain()),
        nice_path(file)
    );
  } else {
    message_box_.message(
        "[500]  Internal Error",
        std::string{error->message()},
        nice_path(file)
    );
  }

  message_box_.show();
}





void image_display::toggle_infobar() {
  if (infobar_.visible()) {
    infobar_.hide();
  } else {
    infobar_.show();
  }
}
