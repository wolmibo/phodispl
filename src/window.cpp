#include "phodispl/window.hpp"

#include "phodispl/config.hpp"
#include "phodispl/font-name.hpp"
#include "phodispl/fonts.hpp"

#include "resources.hpp"

#include <logcerr/log.hpp>

#include <win/modifier.hpp>
#include <win/widget-constraint.hpp>




namespace {
  [[nodiscard]] bool activation_area(const vec2<float>& pos, const vec2<float>& size) {
    return pos.x() < 112.f || pos.x() > size.x() - 122.f;
  }



  template<typename T>
  void assert_eq(T l, T r, std::string_view msg) {
    if (l != r) {
      throw std::runtime_error{"assertion failed: " + std::string{msg}};
    }
  }
}





window::window(std::vector<std::filesystem::path> sl) :
  win::application{"phodispl"},

  image_source_{
    [this](std::shared_ptr<image> img, image_change) {
      image_display_.active(std::move(img));
    },
    std::move(sl), *this
  },

  nav_left_ {true,  [this]() { image_source_.previous_image(); }},
  nav_right_{false, [this]() { image_source_.next_image();     }}
{
  logcerr::verbose("window backend: {}", win::to_string(backend()));

  background_color(global_config().theme_background);

  const auto& font_path = global_config().theme_font.path();
  logcerr::verbose("font file: {}", font_path.native());
  assert_eq(add_font(gl::glyphs{font_path}), font_main,  "font_main index mismatch");
  assert_eq(add_font(gl::glyphs::from_static_memory(resources::icons_font_data())),
      font_icons, "font_icons index mismatch");

  add_child(&image_display_, win::widget_constraint{
      .width  = win::dimension_fill_constraint{},
      .height = win::dimension_fill_constraint{},
      .margin = win::margin_constraint{
        .start  = 0.f,
        .end    = 0.f,
        .top    = 0.f,
        .bottom = 0.f
      }
  });

  add_child(&nav_left_, win::widget_constraint{
      .width  = win::dimension_compute_constraint{},
      .height = win::dimension_compute_constraint{},
      .margin = win::margin_constraint{
        .start  = 24.f,
        .end    = {},
        .top    = 24.f,
        .bottom = 24.f,
      }
  });

  add_child(&nav_right_, win::widget_constraint{
      .width  = win::dimension_compute_constraint{},
      .height = win::dimension_compute_constraint{},
      .margin = win::margin_constraint{
        .start  = {},
        .end    = 24.f,
        .top    = 24.f,
        .bottom = 24.f,
      }
  });

  application::window().min_size({144, 144});
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

  if (input_mode_ == input_mode::standard && mode != input_mode_) {
    zoom_scale_.deactivate();
  } else {
    clear_input_mode(input_mode_);
  }

  input_mode_ = mode;
}





void window::input_mode_scale(continuous_scale::direction direction, bool activate) {
  switch (input_mode_) {
    case input_mode::exposure_control:
      exposure_scale_.set(direction, activate);
      break;
    case input_mode::standard:
      zoom_scale_.set(direction, activate);
      break;
  }
}





void window::on_update() {
  if (auto samp = exposure_scale_.next_sample(); exposure_scale_) {
    image_display_.exposure_multiply(std::pow(1.01f, samp));
  }

  if (auto samp = zoom_scale_.next_sample(); zoom_scale_) {
    image_display_.scale_multiply_at(std::pow(1.01f, samp), 0.5f * logical_size());
  }

  auto samp_x = move_x_scale_.next_sample();
  auto samp_y = move_y_scale_.next_sample();
  if (move_x_scale_ || move_y_scale_) {
    image_display_.translate({samp_x, samp_y});
  }

  update_title();
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
    application::window().title(new_title);
  }
}





void window::on_key_leave() {
  move_x_scale_.deactivate();
  move_y_scale_.deactivate();

  zoom_scale_.deactivate();
  exposure_scale_.deactivate();
}



void window::on_key_release(win::key keycode) {
  switch (keycode) {
    case win::key_from_char('w'):
    case win::key_from_char('W'):
      move_y_scale_.deactivate_up();
      break;
    case win::key_from_char('a'):
    case win::key_from_char('A'):
      move_x_scale_.deactivate_up();
      break;
    case win::key_from_char('s'):
    case win::key_from_char('S'):
      move_y_scale_.deactivate_down();
      break;
    case win::key_from_char('d'):
    case win::key_from_char('D'):
      move_x_scale_.deactivate_down();
      break;

    case win::key::kp_plus:
    case win::key_from_char('+'):
      input_mode_scale(continuous_scale::direction::up, false);
      break;

    case win::key::kp_minus:
    case win::key_from_char('-'):
      input_mode_scale(continuous_scale::direction::down, false);
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
      application::window().close();
      break;

    case win::key::home: scale = -1; break;
    case win::key::end:  scale = -2; break;

    case win::key::kp_0: scale = -1; break;
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
      move_y_scale_.activate_up();
      break;
    case win::key_from_char('a'):
    case win::key_from_char('A'):
      move_x_scale_.activate_up();
      break;
    case win::key_from_char('s'):
    case win::key_from_char('S'):
      move_y_scale_.activate_down();
      break;
    case win::key_from_char('d'):
    case win::key_from_char('D'):
      move_x_scale_.activate_down();
      break;

    case win::key::kp_plus:
    case win::key_from_char('+'):
      input_mode_scale(continuous_scale::direction::up, true);
      break;
    case win::key::kp_minus:
    case win::key_from_char('-'):
      input_mode_scale(continuous_scale::direction::down, true);
      break;

    case win::key_from_char('e'):
    case win::key_from_char('E'):
      set_input_mode(input_mode::exposure_control);
      break;



    case win::key::f5:
    case win::key_from_char('r'):
    case win::key_from_char('R'):
      if (application::window().mod_active(win::modifier::control)) {
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
      image_display_.toggle_scale_filter();
      break;

    case win::key_from_char('i'):
    case win::key_from_char('I'):
      image_display_.toggle_infobar();
      break;

    default:
      break;
  }

  input_mode_apply_scale(scale);
}



void window::input_mode_apply_scale(int scale) {
  if (scale > 0) {
    switch (input_mode_) {
      case input_mode::exposure_control:
        if (application::window().mod_active(win::modifier::control)) {
          image_display_.exposure(std::pow(0.5f, scale));
        } else {
          image_display_.exposure(std::pow(2.f, scale));
        }
        break;
      case input_mode::standard:
        if (application::window().mod_active(win::modifier::control)) {
          image_display_.scale(static_cast<float>(scale) / win::widget::scale());
        } else {
          image_display_.scale(1.f / static_cast<float>(scale) / win::widget::scale());
        }
        break;
    }
  } else if (scale == -1) {
    switch (input_mode_) {
      case input_mode::exposure_control:
        image_display_.exposure(1.f);
        break;
      case input_mode::standard:
        image_display_.scale(dynamic_scale::fit);
        break;
    }
  } else if (scale == -2 && input_mode_ == input_mode::standard) {
    image_display_.scale(dynamic_scale::clip);
  }
}





void window::on_pointer_press(vec2<float> pos, win::mouse_button button) {
  if (button == win::mouse_button::middle) {
    dragging_ = true;
    last_position_ = pos;
  } else if (button == win::mouse_button::left) {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - last_left_click_).count() < 250) {
      dragging_ = true;
      last_position_ = pos;
    }
    last_left_click_ = std::chrono::steady_clock::now();
  }
}



void window::on_pointer_release(vec2<float> /*pos*/, win::mouse_button button) {
  if (button == win::mouse_button::middle || button == win::mouse_button::left) {
    dragging_ = false;
  }
}



void window::on_pointer_move(vec2<float> pos) {
  if (dragging_) {
    image_display_.translate(pos - last_position_);

    last_position_ = pos;
  }



  if (!dragging_ && activation_area(pos, logical_size()) && image_source_) {
    nav_left_.show();
    nav_right_.show();
  }
}





void window::on_scroll(vec2<float> pos, vec2<float> delta) {
  if (abs(delta.y()) < 1e-5) {
    return;
  }

  switch (input_mode_) {
    case input_mode::exposure_control:
      image_display_.exposure_multiply(std::pow(1.01f, delta.y()));
      break;
    case input_mode::standard:
      image_display_.scale_multiply_at(std::pow(1.01f, delta.y()), pos);
      break;
  }
}





void window::on_pinch_begin(vec2<float> pos) {
  last_position_ = pos;
  last_scale_ = 1.f;
}





void window::on_pinch_update(vec2<float> /*delta*/, float scale, float /*rot*/) {
  float ds = scale / last_scale_;
  ds *= ds;
  image_display_.scale_multiply_at(ds, last_position_);
  last_scale_ = scale;
}





void window::on_swipe_begin(vec2<float> /*pos*/, uint32_t fingers) {
  if (fingers == 3) {
    dragging_ = true;
  }
}



void window::on_swipe_update(vec2<float> delta) {
  if (dragging_) {
    image_display_.translate(delta);
  }
}



void window::on_swipe_finish() {
  dragging_ = false;
}
