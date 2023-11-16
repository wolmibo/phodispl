#include "phodispl/message-box.hpp"

#include "phodispl/config.hpp"
#include "phodispl/fonts.hpp"
#include "phodispl/formatting.hpp"

#include "resources.hpp"

#include <codecvt>
#include <locale>

#include <gl/primitives.hpp>

#include <win/viewport.hpp>



message_box::message_box() :
  quad_{gl::primitives::quad()},

  shader_{
    resources::shader_plane_object_vs_sv(),
    resources::shader_plane_solid_fs_sv()
  },

  shader_trafo_{shader_.uniform("transform")},
  shader_color_{shader_.uniform("color")}
{}





namespace {
  [[nodiscard]] std::u32string convert_string(const std::string& str) {
    return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(str);
  }
}



void message_box::message(
    std::string           header,
    std::string           message,
    std::filesystem::path path
) {
  if (header_base_  != header ||
      message_base_ != message ||
      path_base_    != path
  ) {
    header_base_  = std::move(header);
    message_base_ = std::move(message);
    path_base_    = std::move(path);

    new_text_ = true;

    invalidate_layout();
  }
}





void message_box::recalculate_string(float width) {
  new_text_ = false;

  if (header_base_.empty()) {
    header_.clear();
  } else {
    header_ = wrap_text(convert_string(header_base_), width, [&](auto text) {
        return viewport().measure_string(text,
            font_main, global_config().theme_heading_size).x;
    });
  }

  if (message_base_.empty()) {
    body_.clear();
  } else {
    body_ = wrap_text(convert_string(message_base_), width, [&](auto text) {
        return viewport().measure_string(text,
            font_main, global_config().theme_text_size).x;
    });
  }

  if (!path_base_.empty()) {
    if (!body_.empty()) {
      body_ += U"\n\n";
    }

    body_ += shorten_path(path_base_, width, [&](auto text) {
        return viewport().measure_string(text,
            font_main, global_config().theme_text_size).x;
    });
  }
}





void message_box::on_layout(win::vec2<std::optional<float>>& size) {
  if (auto width = size.x; width) {
    recalculate_string(*width);

    float height{0.f};

    height += (std::ranges::count(header_, U'\n') + 1) * viewport().measure_string(U"\n",
                  font_main, global_config().theme_heading_size).y;

    height += global_config().theme_heading_size * 0.4f;

    height += (std::ranges::count(body_, U'\n') + 1)
                 * viewport().measure_string(U"\n",
                     font_main, global_config().theme_text_size).y;

    size.y = height;
  }
}





namespace {
  color set_alpha(color c, float alpha) {
    c[3] *= alpha;
    return c;
  }
}



void message_box::on_render() {
  if (!visible()) {
    return;
  }

  if (new_text_) {
    recalculate_string(logical_size().x);
  }



  win::vec2<float> anchor(0.f, global_config().theme_heading_size);

  anchor.y += draw_string(anchor, header_, font_main, global_config().theme_heading_size,
                set_alpha(global_config().theme_heading_color, opacity())).y
                + global_config().theme_heading_size * 0.4f;

  shader_.use();
  win::set_uniform_mat4(shader_trafo_,
      trafo_mat_logical(anchor, {logical_size().x, 2.f}));

  auto c = set_alpha(global_config().theme_heading_color, opacity() * 0.5f);
  glUniform4f(shader_color_, c[0] * c[3], c[1] * c[3], c[2] * c[3], c[3]);
  quad_.draw();

  anchor.y += global_config().theme_heading_size * 0.8f;

  draw_string(anchor, body_, font_main, global_config().theme_text_size,
      set_alpha(global_config().theme_text_color, opacity()));
}
