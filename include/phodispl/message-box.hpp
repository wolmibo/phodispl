// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_MESSAGE_BOX_HPP_INCLUDED
#define PHODISPL_MESSAGE_BOX_HPP_INCLUDED

#include "phodispl/fade-widget.hpp"

#include <string>

#include <gl/mesh.hpp>
#include <gl/program.hpp>



class message_box : public fade_widget {
  public:
    message_box();



    void message(const std::string&, const std::string&);



  private:
    std::string      header_base_;
    std::u32string   header_;
    std::string      message_base_;
    std::u32string   message_;

    gl::mesh         quad_;
    gl::program      shader_;
    GLint            shader_trafo_;
    GLint            shader_color_;

    bool             new_text_{false};



    void on_render() override;
    void on_layout(win::vec2<std::optional<float>>& /*size*/) override;

    void recalculate_string(float);
};

#endif // PHODISPL_MESSAGE_BOX_HPP_INCLUDED
