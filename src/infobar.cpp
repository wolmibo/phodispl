#include "phodispl/infobar.hpp"
#include "phodispl/fade-widget.hpp"
#include "resources.hpp"

#include <codecvt>
#include <locale>
#include <string>
#include <string_view>

#include <gl/primitives.hpp>

#include <pixglot/codecs.hpp>
#include <pixglot/frame-source-info.hpp>




namespace {
  [[nodiscard]] std::u32string convert_string(std::string_view str) {
    return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
              .from_bytes(str.begin(), str.end());
  }



  [[nodiscard]] std::u32string_view stringify(pixglot::color_model model) {
    using enum pixglot::color_model;
    switch (model) {
      case yuv:     return U"YUV";
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
      output += convert_string(std::to_string(std::to_underlying(fsi.subsampling())));
    }

    if (fsi.has_alpha()) {
      output.push_back(U'A');
    }

    output.push_back(U' ');
    output.push_back(U'(');

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

    output.push_back(U')');

    return output;
  }
}





infobar::infobar() :
  quad_  {gl::primitives::quad()},
  shader_{
    resources::shader_plane_object_vs(),
    resources::shader_plane_solid_fs()
  },
  shader_trafo_{shader_.uniform("transform")},
  shader_color_{shader_.uniform("color")}
{}





void infobar::on_pointer_enter(win::vec2<float> /*pos*/) {
  mouse_inside_ = true;
}



void infobar::on_pointer_leave() {
  mouse_inside_ = false;
  mouse_leave_ = std::chrono::steady_clock::now();
}





void infobar::show() {
  mouse_leave_ = std::chrono::steady_clock::now();

  fade_widget::show();
}





void infobar::on_update() {
  if (!mouse_inside_ &&
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - mouse_leave_).count() > 2) {
    fade_widget::hide();
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
}
