#include "phodispl/box.hpp"
#include "phodispl/config.hpp"
#include "phodispl/frame.hpp"
#include "phodispl/image-view.hpp"
#include "phodispl/view-transform.hpp"
#include "phodispl/viewport.hpp"



view_transform::vt mix(view_transform::vt a, view_transform::vt b, float v) {
  float mv = 1.0 - v;
  return {
    .scale = mv * a.scale + v * b.scale,
    .cx    = mv * a.cx    + v * b.cx,
    .cy    = mv * a.cy    + v * b.cy,
  };
}



box view_transform::to_box() const {
  float w{0.f};
  float h{0.f};

  if (size_) {
    w = (*size_)[0];
    h = (*size_)[1];
  }

  auto tf = *trafo_;

  return {
    .width        = w * tf.scale,
    .height       = h * tf.scale,
    .x            = tf.cx,
    .y            = tf.cy,
    .anchor       = placement::center,
    .align_pixels = false,
  };
}



view_transform::view_transform(std::shared_ptr<view_info> vi) :
  view_info_{std::move(vi)},
  trafo_{
    static_cast<float>(global_config().animation_view_snap_ms.count()),
    global_config().animation_view_snap_interpolation
  }
{}



void view_transform::recalculate(bool immediate) {
  if (!size_) return;

  float width  = (*size_)[0];
  float height = (*size_)[1];

  valid_ = true;

  vt new_trafo = { 0.0, 0.0, 0.0 };
  switch (view_mode_) {
    case view_mode::fit:
      new_trafo.cx    = 0.0;
      new_trafo.cy    = 0.0;
      new_trafo.scale = view_info_->viewport_->scale_fit(width, height);
      break;
    case view_mode::clip:
      new_trafo.cx    = 0.0;
      new_trafo.cy    = 0.0;
      new_trafo.scale = view_info_->viewport_->scale_clip(width, height);
      break;
    case view_mode::custom:
      return;
  }
  trafo_.animate_to(new_trafo, immediate);
}



void view_transform::update(const frame& f) {
  size_ = { f.real_width(), f.real_height() };
  recalculate(true);
}



void view_transform::scale(float s, float x, float y) {
  view_mode_ = view_mode::custom;

  trafo_->cx = (trafo_->cx - x) * s + x;
  trafo_->cy = (trafo_->cy - y) * s + y;
  trafo_->scale *= s;

  trafo_.animate_edited(true);
}



void view_transform::translate(float dx, float dy) {
  view_mode_ = view_mode::custom;

  trafo_->cx -= dx;
  trafo_->cy -= dy;

  trafo_.animate_edited(true);
}



void view_transform::snap_absolute_scale(float s) {
  view_mode_ = view_mode::custom;

  float sf = s / trafo_->scale;
  trafo_->cx    *= sf;
  trafo_->cy    *= sf;
  trafo_->scale  = s;
  trafo_.animate_edited();
}



void view_transform::snap_fit() {
  view_mode_ = view_mode::fit;
  recalculate(false);
}



void view_transform::snap_clip() {
  view_mode_ = view_mode::clip;
  recalculate(false);
}



bool view_transform::take_change() {
  return trafo_.changed();
}



void view_transform::save_state() {
  trafo_saved_     = *trafo_;
  view_mode_saved_ = view_mode_;
}



void view_transform::restore_state() {
  view_mode_ = view_mode_saved_;
  trafo_.animate_to(trafo_saved_, true);

  invalidate();
}
