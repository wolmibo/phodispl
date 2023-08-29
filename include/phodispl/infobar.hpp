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

    void set_frame(const pixglot::frame_view&);
    void set_image(const image&);

    void clear_frame();
    void clear_image();

    void show();



  private:
    gl::mesh                              quad_;
    gl::program                           shader_;
    GLint                                 shader_trafo_;
    GLint                                 shader_color_;

    std::chrono::steady_clock::time_point mouse_leave_;
    bool                                  mouse_inside_{false};

    std::filesystem::path                 image_path_;

    std::u32string                        str_format_;
    std::u32string                        str_size_;
    std::optional<pixglot::codec>         codec_;
    std::u32string                        str_file_size_;
    std::u32string                        str_name_;
    std::u32string                        str_path_;
    std::u32string                        str_warning_count_{U"0"};

    void on_render() override;
    void on_update() override;

    void on_layout(win::vec2<std::optional<float>>& /*size*/) override;

    void on_pointer_enter(win::vec2<float> /*pos*/) override;
    void on_pointer_move (win::vec2<float> /*pos*/) override;
    void on_pointer_leave()                         override;

    void recalculate_strings();
};

#endif // PHODISPL_INFOBAR_HPP_INCLUDED
