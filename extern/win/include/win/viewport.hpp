// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_VIEWPORT_HPP_INCLUDED
#define WIN_VIEWPORT_HPP_INCLUDED

#include "win/widget.hpp"

#include <string_view>

#include <gl/glyphs.hpp>
#include <gl/mesh.hpp>
#include <gl/program.hpp>



namespace win {

class viewport : public widget {
  public:
    viewport(const viewport&) = delete;
    viewport(viewport&&)      = default;
    viewport& operator=(const viewport&) = delete;
    viewport& operator=(viewport&&)      = default;

    ~viewport() override = default;

    viewport();



    [[nodiscard]] color background_color() const { return bg_color_; }
    void background_color(color c) { bg_color_ = c; invalidate(); }



    size_t add_font(gl::glyphs);



    vec2<float> draw_string(vec2<float>, std::u32string_view,
        size_t, uint32_t, color) const;
    vec2<float> measure_string(std::u32string_view, size_t, uint32_t) const;



    void resize(vec2<float>, float);

    void render();



  private:
    mutable gl::program     font_shader_;
    mutable gl::mesh        font_plane_;
    std::vector<gl::glyphs> font_cache_;

    color                   bg_color_{0.f, 0.f, 0.f, 1.f};
};

}

#endif // WIN_VIEWPORT_HPP_INCLUDED
