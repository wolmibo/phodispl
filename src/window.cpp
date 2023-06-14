#include "phodispl/config.hpp"
#include "phodispl/message-box.hpp"
#include "phodispl/progress-circle.hpp"
#include "phodispl/viewport.hpp"
#include "phodispl/window.hpp"

#include <logcerr/log.hpp>



namespace {
  [[nodiscard]] bool assign_compare(float& old_value, float new_value) {
    bool cmp = std::abs(old_value - new_value) > 1e-5;
    old_value = new_value;
    return cmp;
  }
}



window::window(std::vector<std::filesystem::path> sl) :
  win::application     {"phodispl"},

  viewport_            {std::make_shared<viewport>()},

  view_info_           {std::make_shared<view_info>(viewport_)},
  image_source_        {std::move(sl), *this},
  image_view_primary_  {view_info_},
  image_view_secondary_{view_info_},

  image_view_blend_    {
    1.0f,
    static_cast<float>(global_config().animation_view_next_ms.count()),
    global_config().animation_view_next_interpolation
  },

  last_left_click_     {false}
{
  background_color(global_config().theme_background);
  logcerr::verbose("window backend: {}", win::to_string(backend()));
}





void window::clear_input_mode(input_mode mode) {
  switch (mode) {
    case input_mode::exposure_control:
      exposure_scale_.deactivate();
      break;
    default:
      break;
  }

  if (input_mode_ == mode) {
    input_mode_ = input_mode::standard;
  }
}





void window::set_input_mode(input_mode mode) {
  if (mode == input_mode_) {
    return;
  }

  clear_input_mode(input_mode_);

  input_mode_ = mode;
}





void window::on_rescale(win::vec2<uint32_t> size, float scale) {
  viewport_->rescale(size.x, size.y, scale);
  image_view_primary_.trafo().invalidate();
  image_view_secondary_.trafo().invalidate();
}





bool window::on_update() {
  switch (image_source_.take_image_change()) {
    case image_change::none:
      if (!image_view_primary_.view_image()) {
        image_view_primary_.reset_image(image_source_.current());
      }
      break;

    case image_change::reload:
      image_view_primary_.reset_image(image_source_.current());
      break;

    case image_change::previous:
    case image_change::next:
    case image_change::replace_deleted:
      image_view_primary_.reset_image(image_source_.current());
      image_view_secondary_.reset_image(image_source_.take_backup());

      std::swap(image_view_primary_.trafo(), image_view_secondary_.trafo());

      image_view_blend_ = 0.0;
      image_view_blend_.animate_to(2.0);
      break;
  }


  if (last_movement_ > 0) {
    auto mv = continuous_movement_.to_vector();

    uint64_t eps = elapsed();
    float time = (eps - last_movement_) / 500.f;

    mv[0] *= time;
    mv[1] *= time;
    mv[2] = powf(1.002, mv[2] * time);

    IMAGE_VIEW_TRAFO(translate(mv[0], mv[1]));
    IMAGE_VIEW_TRAFO(scale(mv[2]));

    last_movement_ = eps;
  } else if (continuous_movement_) {
    last_movement_ = elapsed();
  }

  if (!continuous_movement_) {
    last_movement_ = 0;
  }



  damage(assign_compare(exposure_,
        exposure_ * powf(1.01f, exposure_scale_.next_sample())));



  damage(image_view_primary_.update());
  damage(image_view_blend_.changed());

  if (!image_view_blend_.is_running()) {
    image_view_secondary_.reset_image(nullptr);
  }

  damage(image_view_secondary_.update());

  update_title();

  return false;
}



void window::on_render() {
  logcerr::debug("rendering");

  if (image_source_) {
    float v = *image_view_blend_;
    image_view_secondary_.render_image(std::clamp(2.f-v, 0.f, 1.f), exposure_);
    image_view_primary_.render_image(std::clamp(v, 0.f, 1.f), exposure_);
  } else {
    image_view_primary_.render_empty();
  }
}





void window::toggle_scale_filter() {
  if (view_info_) {
    if (view_info_->scale_filter_ == scale_filter::linear) {
      view_info_->scale_filter_ = scale_filter::nearest;
    } else {
      view_info_->scale_filter_ = scale_filter::linear;
    }
    damage();
  }
}



void window::update_title() {
  static std::string old_title;

  std::string new_title;
  if (auto current = image_source_.current()) {
    new_title = "PhoDispl - [" + current->path().string() + ":"
      + std::to_string(current->frame_index() + 1) + "/"
      + std::to_string(current->frame_count()) + "]";
  } else {
    new_title = "PhoDispl";
  }

  if (new_title != old_title) {
    old_title = new_title;
    title(new_title);
  }
}





void window::on_key_leave() {
  continuous_movement_.clear();
}



void window::on_key_release(win::key keycode) {
  switch (keycode) {
    case win::key_from_char('w'):
    case win::key_from_char('W'):
      continuous_movement_.reset(movement::direction::up);
      break;
    case win::key_from_char('a'):
    case win::key_from_char('A'):
      continuous_movement_.reset(movement::direction::left);
      break;
    case win::key_from_char('s'):
    case win::key_from_char('S'):
      continuous_movement_.reset(movement::direction::down);
      break;
    case win::key_from_char('d'):
    case win::key_from_char('D'):
      continuous_movement_.reset(movement::direction::right);
      break;

    case win::key::kp_plus:
    case win::key_from_char('+'):
      switch (input_mode_) {
        case input_mode::standard:
          continuous_movement_.reset(movement::direction::in);
          break;
        case input_mode::exposure_control:
          exposure_scale_.deactivate_up();
          break;
      }
      break;

    case win::key::kp_minus:
    case win::key_from_char('-'):
      switch (input_mode_) {
        case input_mode::standard:
          continuous_movement_.reset(movement::direction::out);
          break;
        case input_mode::exposure_control:
          exposure_scale_.deactivate_down();
          break;
      }
      break;

    case win::key_from_char('e'):
    case win::key_from_char('E'):
      clear_input_mode(input_mode::exposure_control);
      break;

    default:
      break;
  }
}



void window::on_key_press(win::key keycode) {
  int32_t scale = 0;

  switch (keycode) {
    case win::key_from_char('q'):
    case win::key_from_char('Q'):
    case win::key::escape:
      close();
      break;

    case win::key::home:
      switch (input_mode_) {
        case input_mode::exposure_control:
          damage(assign_compare(exposure_, 1.f));
          break;
        case input_mode::standard:
          scale = -1;
          break;
      }
      break;
    case win::key::end:  scale = -2; break;

    case win::key::kp_1: scale =  1; break;
    case win::key::kp_2: scale =  2; break;
    case win::key::kp_3: scale =  3; break;
    case win::key::kp_4: scale =  4; break;
    case win::key::kp_5: scale =  5; break;
    case win::key::kp_6: scale =  6; break;
    case win::key::kp_7: scale =  7; break;
    case win::key::kp_8: scale =  8; break;
    case win::key::kp_9: scale =  9; break;

    case win::key_from_char('w'):
    case win::key_from_char('W'):
      continuous_movement_.set(movement::direction::up);
      break;
    case win::key_from_char('a'):
    case win::key_from_char('A'):
      continuous_movement_.set(movement::direction::left);
      break;
    case win::key_from_char('s'):
    case win::key_from_char('S'):
      continuous_movement_.set(movement::direction::down);
      break;
    case win::key_from_char('d'):
    case win::key_from_char('D'):
      continuous_movement_.set(movement::direction::right);
      break;

    case win::key::kp_plus:
    case win::key_from_char('+'):
      switch (input_mode_) {
        case input_mode::exposure_control:
          exposure_scale_.activate_up();
          break;
        case input_mode::standard:
          continuous_movement_.set(movement::direction::in);
          break;
      }
      break;
    case win::key::kp_minus:
    case win::key_from_char('-'):
      switch (input_mode_) {
        case input_mode::exposure_control:
          exposure_scale_.activate_down();
          break;
        case input_mode::standard:
          continuous_movement_.set(movement::direction::out);
          break;
      }
      break;

    case win::key_from_char('e'):
    case win::key_from_char('E'):
      set_input_mode(input_mode::exposure_control);
      break;



    case win::key::f5:
    case win::key_from_char('r'):
    case win::key_from_char('R'):
      if (mod_active(win::modifier::control)) {
        image_source_.reload_file_list();
      } else {
        image_source_.reload_current();
      }
      break;

    case win::key::right:
      image_source_.next_image();
      break;
    case win::key::left:
      image_source_.previous_image();
      break;
    case win::key::up:
      if (auto image = image_source_.current()) {
        image->next_frame();
      }
      break;
    case win::key::down:
      if (auto image = image_source_.current()) {
        image->previous_frame();
      }
      break;

    case win::key_from_char(' '):
      if (auto current = image_source_.current()) {
        current->toggle_animation();
      }
      break;

    case win::key_from_char('t'):
    case win::key_from_char('T'):
      toggle_scale_filter();
      break;

    default:
      break;
  }

  if (scale > 0) {
    float s = scale;
    if (!mod_active(win::modifier::control)) {
      s = 1.f / s;
    }
    IMAGE_VIEW_TRAFO(snap_absolute_scale(s));
  } else if (scale == -1) {
    IMAGE_VIEW_TRAFO(snap_fit());
  } else if (scale == -2) {
    IMAGE_VIEW_TRAFO(snap_clip());
  }
}





void window::on_pointer_press(win::vec2<float> pos, win::mouse_button button) {
  if (button == win::mouse_button::middle) {
    dragging_ = true;
    last_x_ = pos.x;
    last_y_ = pos.y;
  } else if (button == win::mouse_button::left) {
    if (last_left_click_.current_ms() < 250) {
      dragging_ = true;
      last_x_ = pos.x;
      last_y_ = pos.y;
    }
    last_left_click_.reset(true);
  }
}



void window::on_pointer_release(win::vec2<float> /*pos*/, win::mouse_button button) {
  if (button == win::mouse_button::middle || button == win::mouse_button::left) {
    dragging_ = false;
  }
}



void window::on_pointer_move(win::vec2<float> pos) {
  if (dragging_) {
    IMAGE_VIEW_TRAFO(translate(last_x_ - pos.x, pos.y - last_y_));

    last_x_ = pos.x;
    last_y_ = pos.y;
  }
}



void window::on_scroll(win::vec2<float> pos, win::vec2<float> delta) {
  if (abs(delta.y) < 1e-5) {
    return;
  }

  switch (input_mode_) {
    case input_mode::exposure_control:
      damage();
      exposure_ *= powf(1.01, delta.y);
      break;
    case input_mode::standard:
      pos.x = pos.x - width() / 2.;
      pos.y = height() / 2. - pos.y;
      delta.y = powf(1.01, delta.y);
      IMAGE_VIEW_TRAFO(scale(delta.y, pos.x, pos.y));
      break;
  }
}





void window::on_pinch_begin(win::vec2<float> pos) {
  last_x_     = pos.x;
  last_y_     = pos.y;
  last_scale_ = 1.f;
}





void window::on_pinch_update(win::vec2<float> /*delta*/, float scale, float /*rot*/) {
  float ds = scale / last_scale_;
  ds *= ds;
  IMAGE_VIEW_TRAFO(scale(ds, last_x_ - width() / 2., height() / 2. - last_y_));
  last_scale_ = scale;
}



void window::on_swipe_begin(win::vec2<float> /*pos*/, uint32_t fingers) {
  if (fingers == 3) {
    dragging_ = true;
  }
}



void window::on_swipe_update(win::vec2<float> delta) {
  if (dragging_) {
    IMAGE_VIEW_TRAFO(translate(-delta.x, delta.y));
  }
}



void window::on_swipe_cancel() {
  if (!dragging_) {
    IMAGE_VIEW_TRAFO(restore_state());
  }
}



void window::on_swipe_finish() {
  dragging_ = false;
}
