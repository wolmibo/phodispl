#ifndef PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED
#define PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED

#include "phodispl/animation.hpp"

#include <gl/mesh.hpp>
#include <gl/program.hpp>

#include <win/widget.hpp>




class progress_circle : public win::widget {
  public:
    explicit progress_circle();


    void show();
    void hide();

    void value(float);



  private:
    bool             visible_   {false};
    bool             might_hide_{false};
    float            value_     {0.f};
    animation<float> alpha_;

    gl::mesh         quad_;
    gl::program      shader_;
    GLint            shader_progress_;
    GLint            shader_alpha_;
    GLint            shader_trafo_continuous_;
    GLint            shader_trafo_;



    void on_update() override;
    void on_render() override;
};

#endif // PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED
