#include "phodispl/box.hpp"
#include "phodispl/config.hpp"
#include "phodispl/viewport.hpp"

#include "resources.hpp"

#include <codecvt>
#include <filesystem>
#include <locale>

#include <fontconfig/fontconfig.h>

#include <gl/primitives.hpp>

#include <logcerr/log.hpp>



namespace {
  inline void apply_matrix(const mat4& mat) {
    glUniformMatrix4fv(0, 1, GL_FALSE, mat.data());
  }




  struct fc_pattern_destructor {
    void operator()(FcPattern* pat) const { FcPatternDestroy(pat); }
  };

  using fc_pattern = std::unique_ptr<FcPattern, fc_pattern_destructor>;



  [[nodiscard]] auto fc_init() {
    if (FcInit() != FcTrue) {
      throw std::runtime_error{"unable to initialize fontconfig"};
    }

    struct fc_destructor { void operator()(const int* /*dummy*/) const { FcFini(); } };
    static constexpr int non_null_address{0};

    return std::unique_ptr<const int, fc_destructor>{&non_null_address};
  }



  [[nodiscard]] std::filesystem::path resolve_font(const std::string& name) {
    auto fc_library_guard = fc_init();

    static_assert(sizeof(FcChar8) == sizeof(char));
    //NOLINTNEXTLINE(*-reinterpret-cast)
    fc_pattern pattern{FcNameParse(reinterpret_cast<const FcChar8*>(name.c_str()))};
    if (!pattern) {
      throw std::runtime_error{"unable to find font " + name + " (FcNameParse)"};
    }

    if (FcConfigSubstitute(nullptr, pattern.get(), FcMatchPattern) != FcTrue) {
      throw std::runtime_error{"unable to find font " + name + " (FcConfigSubstitute)"};
    }
    FcDefaultSubstitute(pattern.get());

    FcResult result{};
    fc_pattern font{FcFontMatch(nullptr, pattern.get(), &result)};
    if (!font || result != FcResultMatch) {
      throw std::runtime_error{"unable to find font " + name + " (FcFontMatch)"};
    }

    FcChar8* file{nullptr};
    if (FcPatternGetString(font.get(), FC_FILE, 0, &file) != FcResultMatch
        || file == nullptr) {
      throw std::runtime_error{"unable to obtain filename for font " + name};
    }

    //NOLINTNEXTLINE(*-reinterpret-cast)
    std::filesystem::path path{reinterpret_cast<const char*>(file)};

    logcerr::debug("font file: {}", path.string());

    return path;
  }



  [[nodiscard]] std::filesystem::path locate_font(const std::string& name) {
    if (std::filesystem::exists(name) && !std::filesystem::is_directory(name)) {
      return name;
    }

    return resolve_font(name);
  }



  [[nodiscard]] std::optional<gl::glyphs> create_glyphs() {
    try {
      return std::make_optional<gl::glyphs>(locate_font(global_config().theme_font));
    } catch (std::exception& ex) {
      logcerr::error("Failed to initialize glyphs: {}", ex.what());
      return {};
    }
  }
}



viewport::viewport() :
  quad_{gl::primitives::quad()},

  font_shader_{
    resources::shader_plane_uv_vs(),
    resources::shader_plane_colorize_fs()
  },
  font_shader_color_{font_shader_.uniform("color")},

  solid_shader_{
    resources::shader_plane_object_vs(),
    resources::shader_plane_solid_fs()
  },
  solid_shader_color_{solid_shader_.uniform("color")},

  glyphs_{create_glyphs()}
{
  assert_shader_compat(solid_shader_, "SHADER_PLANE_OBJECT_VS");
  assert_shader_compat(font_shader_,  "SHADER_PLANE_UV_VS");
}





void viewport::rescale(uint32_t w, uint32_t h, float s) {
  width_  = w;
  height_ = h;
  scale_  = s;
}



void viewport::render(const box& b) const {
  apply_matrix(b.to_matrix(width_, height_));
  quad_.draw();
}



void viewport::render_physical(const box& b) const {
  auto copy = b;
  copy.x *= scale_;
  copy.y *= scale_;
  apply_matrix(copy.to_matrix(width_ * scale_, height_ * scale_));
  quad_.draw();
}



void viewport::render_solid(const box& b, const color& c) const {
  solid_shader_.use();
  glUniform4f(solid_shader_color_, c[0], c[1], c[2], c[3]);
  render(b);
}



void viewport::render_solid(const color& c) const {
  solid_shader_.use();
  glUniform4f(solid_shader_color_, c[0], c[1], c[2], c[3]);
  apply_matrix(mat4_unity);
  quad_.draw();
}



std::u32string viewport::convert_string(const std::string& str) {
  return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(str);
}



void viewport::draw_string(
    const std::u32string& utf32,
    uint32_t              size,
    float                 x,
    float                 y,
    const color&          col
) {
  if (!glyphs_) return;

  float xoffset = 0;
  float yoffset = 0;

  x = std::floor(x * scale_);
  y = std::floor(y * scale_);

  font_shader_.use();
  glUniform4f(font_shader_color_, col[0], col[1], col[2], col[3]);

  box b = {
    .anchor       = placement::center,
    .align_pixels = true,
  };

  for (char32_t c: utf32) {
    if (c == 10) {
      xoffset = 0;
      yoffset -= 1.2 * size * scale_;
    } else if (c == 32) {
      xoffset += glyphs_->get(c, size * scale_).advance_x();
    } else {
      const auto& g = glyphs_->get(c, size * scale_);

      b.x      = x + g.width()  / 2 + g.left() + xoffset;
      b.y      = y - g.height() / 2 + g.top()  + yoffset;
      b.width  = g.width();
      b.height = g.height();

      xoffset += g.advance_x();

      g.tex().bind();
      apply_matrix(b.to_matrix(width_ * scale_, height_ * scale_));
      quad_.draw();
    }
  }
}



std::pair<float, float> viewport::string_dimensions(
    const std::u32string& utf32,
    uint32_t              size
) {
  if (!glyphs_) {
    return {0.f, 0.f};
  }

  float xoffset = 0;
  float yoffset = size;

  float x = 0;

  for (char32_t c: utf32) {
    if (c == 10) {
      yoffset += 1.2 * size;
      x = std::max(xoffset, x);
      xoffset = 0;
    } else {
      xoffset += glyphs_->get(c, size * scale_).advance_x() / scale_;
    }
  }

  return std::make_pair(std::max(x, xoffset), yoffset);
}



float viewport::scale_fit(float w, float h) const {
  return std::min<float>(width_ / w, height_ / h) * scale_;
}

float viewport::scale_clip(float w, float h) const {
  return std::max<float>(width_ / w, height_ / h) * scale_;
}



void viewport::assert_shader_compat(const gl::program& shader, const std::string& name) {
  if (shader.uniform("transform") != 0) {
    throw std::runtime_error("shader error: missing uniform \"transform\" in " + name);
  }
  if (shader.attribute("position") != 0) {
    throw std::runtime_error("shader error: missing attribute \"position\" in " + name);
  }
}
