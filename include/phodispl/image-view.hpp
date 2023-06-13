#ifndef PHODISPL_IMAGE_VIEW_HPP_INCLUDED
#define PHODISPL_IMAGE_VIEW_HPP_INCLUDED

#include "phodispl/animation.hpp"
#include "phodispl/config-types.hpp"
#include "phodispl/message-box.hpp"
#include "phodispl/progress-circle.hpp"
#include "phodispl/view-transform.hpp"

#include <memory>



class viewport;
class image;


struct view_info {
  std::shared_ptr<viewport> viewport_;

  scale_filter      scale_filter_;
  progress_circle   progress_circle_;
  message_box       message_box_;

  gl::program       shader_color_;
  GLint             shader_color_alpha_{0};
  gl::program       shader_color_gc_;
  GLint             shader_color_gc_alpha_{0};
  GLint             shader_color_gc_exponent_{0};

  explicit view_info(std::shared_ptr<viewport>);
};





class image_view {
  public:
    explicit image_view(std::shared_ptr<view_info>);



    bool update();



    [[nodiscard]] view_transform& trafo() { return view_transform_; }

    [[nodiscard]] const std::shared_ptr<image>& view_image() const { return image_; }

    void reset_image(const std::shared_ptr<image>& = {});

    void render_image(float = 1.f);
    void render_empty();



  private:
    enum class image_state {
      empty,
      loading,
      present,
      clean,
      error,
    };

    [[nodiscard]] static bool image_state_final(image_state is) {
      return is == image_state::clean || is == image_state::error;
    }



    std::shared_ptr<view_info> view_info_;

    std::shared_ptr<image>     image_;
    view_transform             view_transform_;

    image_state                last_image_state_{image_state::empty};
    animation<float>           loading_blend_;



    void render_frame          (const frame&, float);
    void render_image_or_backup(const image&, float);
};

#endif // PHODISPL_IMAGE_VIEW_HPP_INCLUDED
