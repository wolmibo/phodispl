#include "phodispl/infobar.hpp"
#include "phodispl/config.hpp"
#include "phodispl/fade-widget.hpp"
#include "phodispl/fonts.hpp"
#include "phodispl/formatting.hpp"
#include "phodispl/image.hpp"
#include "resources.hpp"

#include <string>
#include <string_view>

#include <gl/primitives.hpp>

#include <pixglot/codecs.hpp>
#include <pixglot/frame.hpp>
#include <pixglot/frame-source-info.hpp>

#include <win/viewport.hpp>



namespace {
  template<typename T>
  [[nodiscard]] bool assign_diff(T& target, T source) {
    if (source != target) {
      target = std::move(source);
      return true;
    }
    return false;
  }



  [[nodiscard]] std::u32string convert_string(std::string_view str) {
    std::u32string output;

    output.resize(str.size());
    std::ranges::copy(str, output.begin());

    return output;
  }



  [[nodiscard]] std::u32string_view stringify(pixglot::color_model model) {
    using enum pixglot::color_model;
    switch (model) {
      case yuv:     return U"yuv";
      case rgb:     return U"RGB";
      case palette: return U"Palette";
      case value:   return U"Value";
      case unknown: break;
    }
    return U"Unknown";
  }



  [[nodiscard]] std::u32string_view stringify(pixglot::codec codec) {
    using enum pixglot::codec;
    switch (codec) {
      case avif: return U"Avif";
      case jpeg: return U"Jpeg";
      case png:  return U"PNG";
      case exr:  return U"EXR";
      case ppm:  return U"PPM";
      case webp: return U"WebP";
      case gif:  return U"Gif";
      case jxl:  return U"Jxl";
    }
    return U"Unknown";
  }



  [[nodiscard]] std::u32string format_fsi(const pixglot::frame_source_info& fsi) {
    std::u32string output;

    output += ::stringify(fsi.color_model());

    if (fsi.color_model() == pixglot::color_model::yuv) {
      output += to_u32string(std::to_underlying(fsi.subsampling()));
    }

    if (fsi.has_alpha()) {
      output.push_back(U'A');
    }

    output.push_back(U' ');

    auto format = fsi.color_model_format();
    output += convert_string(pixglot::stringify(format[0]));

    bool expand = (fsi.has_color() && (format[0] != format[1] || format[0] != format[2]))
                  || (fsi.has_alpha() && (format[0] != format[3]));

    if (expand) {
      if (fsi.has_color()) {
        output += U", ";
        output += convert_string(pixglot::stringify(format[1]));
        output += U", ";
        output += convert_string(pixglot::stringify(format[2]));
      }

      if (fsi.has_alpha()) {
        output += U", ";
        output += convert_string(pixglot::stringify(format[3]));
      }
    }


    return output;
  }



  [[nodiscard]] std::u32string format_size(size_t width, size_t height) {
    return to_u32string(width) + U'×' + to_u32string(height);
  }
}





void infobar::set_frame(const pixglot::frame_view& frame) {
  invalidate(assign_diff(str_format_, format_fsi (frame.source_info())));
  invalidate(assign_diff(str_size_,   format_size(frame.width(), frame.height())));
}



void infobar::clear_frame() {
  invalidate(assign_diff<std::u32string>(str_format_, U""));
  invalidate(assign_diff<std::u32string>(str_size_,   U""));
}





void infobar::set_image(const image& img) {
  invalidate(assign_diff(str_path_, nice_path(img.path().parent_path()).u32string()));
  invalidate(assign_diff(str_name_, img.path().filename().u32string()));

  if (img.loading() || img.finished()) {
    invalidate(assign_diff(codec_, img.codec()));
    invalidate(assign_diff(str_file_size_, format_byte_size(img.file_size())));
    invalidate(assign_diff(str_warning_count_, to_u32string(img.warnings().size())));
  } else {
    invalidate(assign_diff(codec_, {}));
    invalidate(assign_diff<std::u32string>(str_file_size_, U""));
    invalidate(assign_diff<std::u32string>(str_warning_count_, U""));
  }
}



void infobar::clear_image() {
  invalidate(assign_diff<std::u32string>(str_path_, U""));
  invalidate(assign_diff<std::u32string>(str_name_, U""));
  invalidate(assign_diff<std::u32string>(str_file_size_, U""));
  invalidate(assign_diff<std::u32string>(str_warning_count_, U""));
  invalidate(assign_diff(codec_, {}));
}





infobar::infobar() :
  quad_  {gl::primitives::quad()},
  shader_{
    resources::shader_plane_object_vs_sv(),
    resources::shader_plane_solid_fs_sv()
  },
  shader_trafo_{shader_.uniform("transform")},
  shader_color_{shader_.uniform("color")}
{}





void infobar::on_pointer_enter(win::vec2<float> /*pos*/) {
  mouse_leave_ = std::chrono::steady_clock::now();

  fade_widget::show();

  mouse_inside_ = true;
}



void infobar::on_pointer_leave() {
  mouse_inside_ = false;
  mouse_leave_ = std::chrono::steady_clock::now();
}





void infobar::on_update() {
  if (!mouse_inside_ &&
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - mouse_leave_).count() > 2) {
    fade_widget::hide();
  }
}





void infobar::on_layout(win::vec2<std::optional<float>>& size) {
  size.y = global_config().theme_text_size * 3.5f;
}





namespace {
  [[nodiscard]] color premultiply(color c, float alpha) {
    return {c[0] * c[3] * alpha, c[1] * c[3] * alpha, c[2] * c[3] * alpha, c[3] * alpha};
  }



  void print(
      std::u32string_view  key,
      std::u32string_view  value,
      win::vec2<float>&    position,
      const win::viewport& viewport,
      float                alpha
  ) {
    viewport.draw_string(position, key, font_icons, global_config().theme_text_size,
        premultiply(global_config().theme_text_color, 0.75 * alpha));

    position += win::vec2<float>{global_config().theme_text_size * 1.2f, 0.f};

    viewport.draw_string(position, value,
        font_main, global_config().theme_text_size,
        premultiply(global_config().theme_text_color, alpha));

    position += win::vec2<float>{global_config().theme_text_size * 6.f, 0.f};
  }
}





void infobar::on_render() {
  if (!visible()) {
    return;
  }

  shader_.use();
  glUniform4f(shader_color_, 0.f, 0.f, 0.f, 0.7f * opacity());

  win::set_uniform_mat4(shader_trafo_,
      trafo_mat_logical({0.f, 0.f}, logical_size()));

  quad_.draw();

  auto line1 = logical_position() + static_cast<float>(global_config().theme_text_size)
                                      * win::vec2<float>{1.0f, 1.5f};

  auto line2 = line1 + static_cast<float>(global_config().theme_text_size)
                         * win::vec2<float>{0.f, 1.25f};

  auto diffx = std::max(
    viewport().draw_string(line1, str_name_,
        font_main, global_config().theme_text_size,
        premultiply(global_config().theme_text_color, opacity())).x,
    viewport().draw_string(line2, str_path_,
        font_main, global_config().theme_text_size,
        premultiply(global_config().theme_text_color, 0.75f * opacity())).x
  ) + global_config().theme_text_size * 1.f;

  line1.x += diffx;
  line2.x += diffx;

  if (str_warning_count_ != U"0") {
    auto shift = viewport().draw_string(line1, U"0",
        font_icons, global_config().theme_text_size,
        premultiply(global_config().theme_text_color, 0.75 * opacity()));

    auto shiftx = shift.x;

    if (str_warning_count_ != U"1") {
      shift += line1 + win::vec2<float>{0.f, global_config().theme_text_size * -0.33f};

      shiftx += viewport().draw_string(shift, str_warning_count_,
          font_main, global_config().theme_text_size * 0.66,
          premultiply(global_config().theme_text_color, opacity())).x;
    }

    shiftx += global_config().theme_text_size;

    line1.x += shiftx;
    line2.x += shiftx;
  }

  if (codec_) {
    print(U"C", ::stringify(*codec_), line1, viewport(), opacity());
    print(U"D", str_file_size_, line2, viewport(), opacity());
    print(U"A", str_size_, line1, viewport(), opacity());
    print(U"B", str_format_, line2, viewport(), opacity());
  }

}
