// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_NAV_BUTTON_HPP_INCLUDED
#define PHODISPL_NAV_BUTTON_HPP_INCLUDED

#include "phodispl/fade-widget.hpp"
#include "win/mouse-button.hpp"

#include <chrono>

#include <gl/mesh.hpp>
#include <gl/program.hpp>



class nav_button : public fade_widget {
  public:
    nav_button(std::move_only_function<void(void)>);



    void show();



  private:
    gl::mesh                              quad_;
    gl::program                           shader_;
    GLint                                 shader_color_;
    GLint                                 shader_trafo_;

    std::chrono::steady_clock::time_point last_movement_;

    std::move_only_function<void(void)>   on_click_;

    enum class state {
      normal,
      hover,
      down,
    }                mouse_state_{state::normal};


    void on_render() override;
    void on_update() override;


    void on_pointer_enter  (win::vec2<float> /*pos*/)                            override;
    void on_pointer_leave  ()                                                    override;
    void on_pointer_press  (win::vec2<float> /*pos*/, win::mouse_button /*btn*/) override;
    void on_pointer_release(win::vec2<float> /*pos*/, win::mouse_button /*btn*/) override;
};

#endif // PHODISPL_NAV_BUTTON_HPP_INCLUDED
