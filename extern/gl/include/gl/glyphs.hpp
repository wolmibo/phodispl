#ifndef GL_GLYPHS_HPP_INCLUDED
#define GL_GLYPHS_HPP_INCLUDED

#include "gl/texture.hpp"

#include <filesystem>
#include <memory>

#include <chaos-map.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H



namespace gl {

class glyph {
  public:
    glyph(const FT_Face&, char32_t, uint32_t);



    [[nodiscard]] const texture& tex() const { return texture_;   }

    [[nodiscard]] float left()         const { return left_;      }
    [[nodiscard]] float top()          const { return top_;       }

    [[nodiscard]] float advance_x()    const { return advance_x_; }
    [[nodiscard]] float advance_y()    const { return advance_y_; }

    [[nodiscard]] float width()        const { return width_;     }
    [[nodiscard]] float height()       const { return height_;    }



  private:
    float   left_;
    float   top_;
    float   advance_x_;
    float   advance_y_;

    float   width_;
    float   height_;

    texture texture_;

};





class glyphs {
  public:
    explicit glyphs(const std::filesystem::path&);



    [[nodiscard]] const glyph& get(char32_t, uint32_t) const;



  private:
    struct lib_destructor  { void operator()(FT_Library) noexcept; };
    struct face_destructor { void operator()(FT_Face)    noexcept; };

    static_assert(std::is_same_v<FT_LibraryRec_*, FT_Library>);
    static_assert(std::is_same_v<FT_FaceRec_*,    FT_Face>);

    std::unique_ptr<FT_LibraryRec_, lib_destructor > library_;
    std::unique_ptr<FT_FaceRec_,    face_destructor> face_;

    mutable chaos_map<uint32_t, chaos_map<char32_t, glyph>> cache_;
};

}

#endif // GL_GLYPHS_HPP_INCLUDED
