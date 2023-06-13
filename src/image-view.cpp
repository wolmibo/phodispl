#include "phodispl/config.hpp"
#include "phodispl/image-view.hpp"
#include "phodispl/image.hpp"
#include "phodispl/viewport.hpp"

#include "resources.hpp"



view_info::view_info(std::shared_ptr<viewport> vp) :
  viewport_       {std::move(vp)},
  scale_filter_   {global_config().filter},
  progress_circle_{viewport_},
  message_box_    {viewport_},

  shader_color_{
    resources::shader_plane_uv_vs(),
    resources::shader_plane_fs()
  },
  shader_color_factor_{shader_color_.uniform("factor")},

  shader_color_gc_{
    resources::shader_plane_uv_vs(),
    resources::shader_plane_gc_fs()
  },
  shader_color_gc_factor_  {shader_color_gc_.uniform("factor")},
  shader_color_gc_exponent_{shader_color_gc_.uniform("exponent")}
{
  viewport::assert_shader_compat(shader_color_,    "SHADER_PLANE_UV_VS");
  viewport::assert_shader_compat(shader_color_gc_, "SHADER_PLANE_UV_VS");
}





image_view::image_view(std::shared_ptr<view_info> vi) :
  view_info_     {std::move(vi)},
  view_transform_{view_info_},
  loading_blend_ {
    0.f,
    global_config().animation_view_next_ms.count() / 2.f,
    global_config().animation_view_next_interpolation
  }
{}



void image_view::render_empty() {
  view_info_->message_box_.render(
    "[204]  No Content",

    "You did not start PhoDispl with any images to load.\n"
    "\n"
    "Make sure to pass an image file or a directory containing\n"
    "image files as argument to PhoDispl.\n"
  );
}



void image_view::render_frame(const frame& frame, float alpha) {
  if (!view_transform_.valid()) {
    view_transform_.update(frame);
  }
  auto b = view_transform_.to_box();

  b.orientation = frame.orientation();

  frame.texture().bind();

  if (std::abs(frame.gamma() - global_config().gamma) < 0.2) {
    view_info_->shader_color_.use();
    glUniform4f(view_info_->shader_color_factor_, alpha, alpha, alpha, alpha);
  } else {
    view_info_->shader_color_gc_.use();
    glUniform4f(view_info_->shader_color_gc_factor_, alpha, alpha, alpha, alpha);

    float exp = frame.gamma() / global_config().gamma;
    glUniform4f(view_info_->shader_color_gc_exponent_, exp, exp, exp, 1.f);
  }

  if (view_info_->scale_filter_ == scale_filter::linear) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  view_info_->viewport_->render_physical(b);
}



void image_view::render_image_or_backup(const image& img, float alpha) {
  if (auto frame = img.current_frame(); frame && frame->texture()) {
    render_frame(*frame, alpha * (1.f - 0.5f * *loading_blend_));

  }
}



void image_view::render_image(float alpha) {
  if (!image_) {
    last_image_state_ = image_state::empty;
    return;
  }

  if (const auto* error = image_->error()) {
    view_info_->message_box_.render(*error, image_->path(), alpha);
    last_image_state_ = image_state::error;
  } else if (image_->loading()) {
    if (last_image_state_ != image_state::loading) {
      loading_blend_.animate_to(1.f, false);
    }

    view_info_->progress_circle_.render(image_->progress(), alpha * *loading_blend_);
    last_image_state_ = image_state::loading;
  } else {
    if (last_image_state_ != image_state::present
      && last_image_state_ != image_state::clean) {

      loading_blend_.animate_to(0.f, false);
    }

    render_image_or_backup(*image_, alpha);

    if (loading_blend_.is_running()) {
      view_info_->progress_circle_.render(1.f, alpha * *loading_blend_);
      last_image_state_ = image_state::present;
    } else {
      last_image_state_ = image_state::clean;
    }
  }
}



void image_view::reset_image(const std::shared_ptr<image>& img) {
  image_ = img;
  view_transform_.invalidate();
  last_image_state_ = image_state::empty;
}



bool image_view::update() {
  bool collected = view_transform_.take_change();

  if (image_) {
    image_->update();
    collected = image_->take_damage()
      || !image_state_final(last_image_state_)
      || collected;
  }

  return collected || loading_blend_.is_running();
}
