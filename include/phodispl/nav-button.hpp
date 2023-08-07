// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_NAV_BUTTON_HPP_INCLUDED
#define PHODISPL_NAV_BUTTON_HPP_INCLUDED

#include "phodispl/fade-widget.hpp"

#include <chrono>

#include <gl/mesh.hpp>
#include <gl/program.hpp>



class nav_button : public fade_widget {
  public:
    nav_button();



    void show();



  private:
    gl::mesh         quad_;
    gl::program      shader_;
    GLint            shader_color_;
    GLint            shader_trafo_;

    std::chrono::steady_clock::time_point
                     last_movement_;



    void on_render()    override;
    void on_update_fw() override;
};

#endif // PHODISPL_NAV_BUTTON_HPP_INCLUDED
