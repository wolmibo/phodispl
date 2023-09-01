// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_NAV_BUTTON_HPP_INCLUDED
#define PHODISPL_NAV_BUTTON_HPP_INCLUDED

#include "phodispl/fade-widget.hpp"

#include <chrono>

#include <gl/mesh.hpp>
#include <gl/program.hpp>

#include <win/mouse-button.hpp>



class nav_button : public fade_widget {
  public:
    nav_button(bool, std::move_only_function<void(void)>);



    void show();



  private:
    bool                                  left_;
    gl::mesh                              quad_;
    gl::program                           shader_;
    GLint                                 shader_color_back_;
    GLint                                 shader_color_front_;
    GLint                                 shader_trafo_;
    GLint                                 shader_scale_x_;
    GLint                                 shader_scale_r_;
    GLint                                 shader_aa_size_;

    std::chrono::steady_clock::time_point last_movement_;
    animation<float>                      highlight_;
    bool                                  mouse_down_{false};

    std::move_only_function<void(void)>   on_click_;


    void on_render() override;
    void on_update() override;
    void on_layout(win::vec2<std::optional<float>>& /*size*/) override;

    [[nodiscard]] bool stencil(win::vec2<float> /*pos*/) const override;

    void on_pointer_enter  (win::vec2<float> /*pos*/)                            override;
    void on_pointer_leave  ()                                                    override;
    void on_pointer_press  (win::vec2<float> /*pos*/, win::mouse_button /*btn*/) override;
    void on_pointer_release(win::vec2<float> /*pos*/, win::mouse_button /*btn*/) override;
};

#endif // PHODISPL_NAV_BUTTON_HPP_INCLUDED
