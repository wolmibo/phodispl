#ifndef PHODISPL_VIEWPORT_HPP_INCLUDED
#define PHODISPL_VIEWPORT_HPP_INCLUDED

#include "phodispl/config-types.hpp"

#include <optional>
#include <string>

#include <gl/glyphs.hpp>
#include <gl/mesh.hpp>
#include <gl/program.hpp>



struct box;

class config;

class viewport {
  public:
    viewport();



    void rescale(uint32_t, uint32_t, uint32_t);
    static void clear();



    void render(const box&) const;
    void render_solid(const box&, const color&) const;
    void render_solid(const color&) const;



    [[nodiscard]] float scale_fit (float w, float h) const;
    [[nodiscard]] float scale_clip(float w, float h) const;

    [[nodiscard]] static std::u32string   convert_string(const std::string&);
    [[nodiscard]] std::pair<float, float> string_dimensions(const std::u32string&,
                                                              uint32_t);
    void draw_string(const std::u32string&, uint32_t, float, float, const color&);


    static void assert_shader_compat(const gl::program&, const std::string&);



  private:
    gl::mesh                  quad_;

    gl::program               font_shader_;
    GLint                     font_shader_color_ {-1};
    gl::program               solid_shader_;
    GLint                     solid_shader_color_{-1};

    std::optional<gl::glyphs> glyphs_;



    float           width_ {0.f};
    float           height_{0.f};
    float           scale_ {1.f};
};

#endif // PHODISPL_VIEWPORT_HPP_INCLUDED
