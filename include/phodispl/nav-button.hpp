// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_NAV_BUTTON_HPP_INCLUDED
#define PHODISPL_NAV_BUTTON_HPP_INCLUDED

#include "phodispl/animation.hpp"

#include <chrono>

#include <gl/mesh.hpp>
#include <gl/program.hpp>

#include <win/widget.hpp>



class nav_button : public win::widget {
  public:
    nav_button();



    void show();


  
  private:
    bool             visible_   {false};
    bool             might_hide_{false};
    animation<float> alpha_;

    gl::mesh         quad_;
    gl::program      shader_;
    GLint            shader_color_;
    GLint            shader_trafo_;

    std::chrono::steady_clock::time_point
                     last_movement_;



    void on_render() override;
    void on_update() override;
};

#endif // PHODISPL_NAV_BUTTON_HPP_INCLUDED
