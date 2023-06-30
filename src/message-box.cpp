#include "phodispl/config.hpp"
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
  header_  = convert_string(header);
  message_ = convert_string(message);

  invalidate();

  if (visible_) {
    return;
  }

  alpha_.animate_to(1.f);

  visible_    = true;
  might_hide_ = false;
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





void message_box::on_render() {
  if (!visible_) {
    return;
  }
}
