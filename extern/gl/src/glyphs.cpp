#include "gl/glyphs.hpp"
#include "freetype/freetype.h"
#include "gl/texture.hpp"

#include <logcerr/log.hpp>



namespace {
  [[nodiscard]] std::string format_error(FT_Error error) {
    if (const char* msg = FT_Error_String(error); msg != nullptr && *msg != 0) {
      return logcerr::format("{} (Error code {})", msg, error);
    }

    return logcerr::format("Error code {}", error);
  }



  void ft_assert(FT_Error e, std::string&& message) {
    if (e != FT_Err_Ok) {
      throw std::runtime_error{message + ":\n" + format_error(e)};
    }
  }



  void ft_check(FT_Error e, std::string_view message) {
    if (e != FT_Err_Ok) {
      logcerr::error("{}:\n{}", message, format_error(e));
    }
  }





  [[nodiscard]] FT_Library create_library() {
    FT_Library lib{nullptr};
    ft_assert(FT_Init_FreeType(&lib), "Unable to initialize FreeType");
    return lib;
  }



  [[nodiscard]] FT_Face create_face(
      FT_Library                   library,
      const std::filesystem::path& path
  ) {
    if (!std::filesystem::exists(path)) {
       throw std::runtime_error{"Cannot load font: file \"" + path.string()
                                   + "\" not found"};
    }

    if (std::filesystem::is_directory(path)) {
       throw std::runtime_error{"Cannot load font: \"" + path.string()
                                   + "\" is a directory"};
    }

    FT_Face face{nullptr};
    ft_assert(FT_New_Face(library, path.c_str(), 0, &face),
        "Unable to initialize font face \"" + path.string() + "\"");
    return face;
  }



  [[nodiscard]] FT_Face create_face(
      FT_Library                 library,
      std::span<const std::byte> data
  ) {
    FT_Face face{nullptr};

    static_assert(sizeof(FT_Byte) == sizeof(std::byte));
    // NOLINTNEXTLINE(*-reinterpret-cast)
    const auto* ptr = reinterpret_cast<const FT_Byte*>(data.data());

    ft_assert(FT_New_Memory_Face(library, ptr, data.size(), 0, &face),
        "Unable to initialize font face from memory (" + std::to_string(data.size()) +
        " bytes)");
    return face;
  }
}





void gl::glyphs::lib_destructor::operator()(FT_Library library) noexcept {
  ft_check(FT_Done_FreeType(library), "Unable to destroy FT_Library");
}

void gl::glyphs::face_destructor::operator()(FT_Face face) noexcept {
  ft_check(FT_Done_Face(face), "Unable to destroy FT_Face");
}





gl::glyphs::glyphs(const std::filesystem::path& path) :
  library_{create_library()},
  face_   {create_face(library_.get(), path)}
{}



gl::glyphs::glyphs(std::span<const std::byte> data) :
  library_{create_library()},
  face_   {create_face(library_.get(), data)}
{}



gl::glyphs gl::glyphs::from_static_memory(std::span<const std::byte> data) {
  return glyphs{data};
}





const gl::glyph& gl::glyphs::get(char32_t code, uint32_t size) const {
  return cache_.find_or_create(size).find_or_create(code, face_.get(), code, size);
}





gl::glyph::glyph(const FT_Face& face, char32_t code, uint32_t size) {
  ft_assert(FT_Set_Pixel_Sizes(face, 0, size), "Unable to set font pixel size");

  auto index = FT_Get_Char_Index(face, code);
  ft_assert(FT_Load_Glyph(face, index, FT_LOAD_DEFAULT), "Unable to load glyph");
  if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
    ft_assert(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL),
              "Unable to render glyph");
  }



  auto w        = face->glyph->bitmap.width;
  auto h        = face->glyph->bitmap.rows;
  size_t stride = std::abs(face->glyph->bitmap.pitch);

  size_t stride_aligned = stride;
  if (stride_aligned % 4 != 0) {
    stride_aligned += 4 - stride_aligned % 4;
  }

  std::vector<uint8_t> pixels(stride_aligned * h);

  std::span source{face->glyph->bitmap.buffer, stride * h};
  std::span target{pixels};

  for (uint32_t y = 0; y < h; ++y) {
    std::ranges::copy(
        source.subspan(y * stride, w),
        target.subspan(y * stride_aligned, w).begin()
    );
  }



  left_      = face->glyph->bitmap_left;
  top_       = face->glyph->bitmap_top;
  advance_x_ = face->glyph->advance.x >> 6;
  advance_y_ = face->glyph->advance.y >> 6;
  width_     = w;
  height_    = h;



  GLuint id{};
  glGenTextures(1, &id);
  texture_ = texture{id};

  texture_.bind();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED,
    GL_UNSIGNED_BYTE, pixels.data());
}
