// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_VIEWPORT_HPP_INCLUDED
#define WIN_VIEWPORT_HPP_INCLUDED

#include "win/types.hpp"
#include "win/widget.hpp"



namespace win {

class viewport : public widget {
  public:
    viewport(const viewport&) = delete;
    viewport(viewport&&)      = delete;
    viewport& operator=(const viewport&) = delete;
    viewport& operator=(viewport&&)      = delete;

    ~viewport() override = default;

    viewport() = default;



    [[nodiscard]] color background_color() const { return bg_color_; }
    void background_color(color c) { bg_color_ = c; invalidate(); }




    void resize(vec2<float>, float);

    void render();



  private:
    color bg_color_{0.f, 0.f, 0.f, 1.f};
};

}

#endif // WIN_VIEWPORT_HPP_INCLUDED
