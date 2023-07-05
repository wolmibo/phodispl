// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_MESSAGE_BOX_HPP_INCLUDED
#define PHODISPL_MESSAGE_BOX_HPP_INCLUDED

#include "phodispl/animation.hpp"

#include <string>

#include <gl/mesh.hpp>
#include <gl/program.hpp>

#include <win/widget.hpp>



class message_box : public win::widget {
  public:
    message_box();



    void message(const std::string&, const std::string&);

    void show();
    void hide();



  private:
    bool             visible_   {false};
    bool             might_hide_{false};
    animation<float> alpha_;

    std::string      header_base_;
    std::u32string   header_;
    std::string      message_base_;
    std::u32string   message_;

    gl::mesh         quad_;
    gl::program      shader_;
    GLint            shader_trafo_;
    GLint            shader_color_;

    bool             new_text_{false};



    void on_update() override;
    void on_render() override;
    void on_layout() override;

    void recalculate_string();
};

#endif // PHODISPL_MESSAGE_BOX_HPP_INCLUDED
