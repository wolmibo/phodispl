// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_INFOBAR_HPP_INCLUDED
#define PHODISPL_INFOBAR_HPP_INCLUDED

#include "phodispl/fade-widget.hpp"

#include <gl/mesh.hpp>
#include <gl/program.hpp>

#include <pixglot/codecs.hpp>

namespace pixglot {
  class frame_view;
}

class image;



class infobar : public fade_widget {
  public:
    infobar();

    void show();

    void set_frame(const pixglot::frame_view&);
    void set_image(const image&);



  private:
    gl::mesh                              quad_;
    gl::program                           shader_;
    GLint                                 shader_trafo_;
    GLint                                 shader_color_;

    std::chrono::steady_clock::time_point mouse_leave_;
    bool                                  mouse_inside_{false};

    std::u32string                        str_format_;
    std::u32string                        str_size_;
    pixglot::codec                        codec_;
    std::u32string                        str_name_;
    std::u32string                        str_path_;

    void on_render() override;
    void on_update() override;

    void on_pointer_enter(win::vec2<float> /*pos*/) override;
    void on_pointer_leave()                         override;
};

#endif // PHODISPL_INFOBAR_HPP_INCLUDED
