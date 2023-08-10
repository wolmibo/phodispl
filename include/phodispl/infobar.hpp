// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_INFOBAR_HPP_INCLUDED
#define PHODISPL_INFOBAR_HPP_INCLUDED

#include "phodispl/fade-widget.hpp"

#include <gl/mesh.hpp>
#include <gl/program.hpp>



class infobar : public fade_widget {
  public:
    infobar();

    void show();



  private:
    gl::mesh                              quad_;
    gl::program                           shader_;
    GLint                                 shader_trafo_;
    GLint                                 shader_color_;

    std::chrono::steady_clock::time_point mouse_leave_;
    bool                                  mouse_inside_{false};

    void on_render() override;
    void on_update() override;

    void on_pointer_enter(win::vec2<float> /*pos*/) override;
    void on_pointer_leave()                         override;
};

#endif // PHODISPL_INFOBAR_HPP_INCLUDED
