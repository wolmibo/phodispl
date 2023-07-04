#include "phodispl/config.hpp"
#include "phodispl/formatting.hpp"
#include "phodispl/message-box.hpp"
#include "resources.hpp"

#include <codecvt>
#include <locale>

#include <gl/primitives.hpp>

#include <win/viewport.hpp>



message_box::message_box() :
  alpha_(
    0.f,
    global_config().animation_view_next_ms.count(),
    global_config().animation_view_next_curve
  ),

  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_plane_object_vs(),
    resources::shader_plane_solid_fs()
  },

  shader_trafo_{shader_.uniform("transform")},
  shader_color_{shader_.uniform("color")}
{}





namespace {
  [[nodiscard]] std::u32string convert_string(const std::string& str) {
    return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(str);
  }
}



void message_box::show_message(const std::string& header, const std::string& message) {
  if (header_base_ != header || message_base_ != message) {
    header_base_  = header;
    message_base_ = message;

    new_text_ = true;

    invalidate();
  }

  if (visible_) {
    return;
  }

  alpha_.animate_to(1.f);

  visible_    = true;
  might_hide_ = false;
}





void message_box::recalculate_string() {
  new_text_ = false;

  if (header_base_.empty()) {
    header_.clear();
  } else {
    header_ = wrap_text(convert_string(header_base_), logical_size().x, [&](auto text) {
        return viewport().measure_string(text, global_config().theme_heading_size).x;
    });
  }

  if (message_base_.empty()) {
    message_.clear();
  } else {
    message_ = wrap_text(convert_string(message_base_), logical_size().x, [&](auto text) {
        return viewport().measure_string(text, global_config().theme_text_size).x;
    });
  }
}





void message_box::on_layout() {
  recalculate_string();
}





void message_box::hide() {
  if (!visible_ || might_hide_) {
    return;
  }

  alpha_.animate_to(0.f);

  might_hide_ = true;
}





void message_box::on_update() {
  if (might_hide_ && *alpha_ < 1e-4f) {
    might_hide_ = false;
    visible_    = false;
    alpha_.set_to(0.f);
    invalidate();
  }

  if (alpha_.changed()) {
    invalidate();
  }
}





namespace {
  color set_alpha(color c, float alpha) {
    c[3] *= alpha;
    return c;
  }
}



void message_box::on_render() {
  if (!visible_) {
    return;
  }

  if (new_text_) {
    recalculate_string();
  }



  win::vec2<float> anchor{0.f, logical_size().y - global_config().theme_heading_size};
 
  anchor.y -= draw_string(anchor, header_, global_config().theme_heading_size,
                set_alpha(global_config().theme_heading_color, *alpha_)).y
                + global_config().theme_heading_size * 0.4f;

  shader_.use();
  win::set_uniform_mat4(shader_trafo_,
      trafo_mat_logical(anchor, {logical_size().x, 2.f}));

  auto c = set_alpha(global_config().theme_heading_color, *alpha_);
  glUniform4f(shader_color_, c[0] * c[3], c[1] * c[3], c[2] * c[3], c[3]);
  quad_.draw();

  anchor.y -= global_config().theme_heading_size * 0.8f;

  draw_string(anchor, message_, global_config().theme_text_size,
      set_alpha(global_config().theme_text_color, *alpha_));
}
