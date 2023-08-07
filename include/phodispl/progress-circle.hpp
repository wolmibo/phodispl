#ifndef PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED
#define PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED

#include "phodispl/fade-widget.hpp"

#include <gl/mesh.hpp>
#include <gl/program.hpp>



class progress_circle : public fade_widget {
  public:
    explicit progress_circle();


    void value(float);



  private:
    float            value_     {0.f};

    gl::mesh         quad_;
    gl::program      shader_;
    GLint            shader_progress_;
    GLint            shader_alpha_;
    GLint            shader_trafo_continuous_;
    GLint            shader_trafo_;



    void on_update_fw() override;
    void on_render()    override;
};

#endif // PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED
