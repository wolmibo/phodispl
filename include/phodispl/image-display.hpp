// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_IMAGE_DISPLAY_HPP_INCLUDED
#define PHODISPL_IMAGE_DISPLAY_HPP_INCLUDED

#include "phodispl/animation.hpp"
#include "phodispl/config-types.hpp"
#include "phodispl/image.hpp"
#include "phodispl/progress-circle.hpp"

#include <gl/mesh.hpp>
#include <gl/program.hpp>

#include <win/widget.hpp>



enum class dynamic_scale {
  fit,
  clip,
};

using scale_mode = std::variant<dynamic_scale, float>;



class image_display : public win::widget {
  public:
    image_display();

    void active(std::shared_ptr<image>);

    void exposure(float);
    void exposure_multiply(float);

    void scale_filter_toggle();

    void scale(scale_mode);
    void scale_multiply_at(float, win::vec2<float>);

    void translate(win::vec2<float>);



  private:
    std::shared_ptr<image>      current_;
    std::shared_ptr<image>      previous_;

    animation_time              crossfade_;

    gl::mesh                    quad_;
    gl::program                 shader_;
    GLint                       shader_factor_a_;
    GLint                       shader_factor_b_;
    GLint                       shader_transform_a_;
    GLint                       shader_transform_b_;

    animation<float>            exposure_;
    scale_filter                scale_filter_;

    scale_mode                  scale_mode_       {dynamic_scale::fit};
    scale_mode                  scale_mode_target_{dynamic_scale::fit};
    animation<win::vec2<float>> position_;

    progress_circle             progress_circle_;



    void on_update() override;
    void on_render() override;



    [[nodiscard]] float     current_scale(scale_mode)            const;
    [[nodiscard]] float     scale_any    (frame&, scale_mode)    const;
    [[nodiscard]] float     scale_dynamic(frame&, dynamic_scale) const;
    [[nodiscard]] win::mat4 matrix_for   (frame&)                const;
};

#endif // PHODISPL_IMAGE_DISPLAY_HPP_INCLUDED
