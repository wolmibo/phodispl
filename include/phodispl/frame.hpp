#ifndef PHODISPL_FRAME_HPP_INCLUDED
#define PHODISPL_FRAME_HPP_INCLUDED

#include <pixglot/frame.hpp>



class frame {
  public:
    explicit frame(pixglot::frame&& frame) :
      or_     {frame.orientation},
      texture_{std::move(frame.pixels.texture())}
    {}



    [[nodiscard]] pixglot::square_isometry orientation() const {
      return or_;
    }

    [[nodiscard]] pixglot::pixel_format format() const {
      return texture_.format();
    }

    [[nodiscard]] const pixglot::gl_texture& texture() const {
      return texture_;
    }



    [[nodiscard]] float real_width() const {
      if (pixglot::flips_xy(or_)) {
        return texture_.height();
      }
      return texture_.width();
    }



    [[nodiscard]] float real_height() const {
      if (pixglot::flips_xy(or_)) {
        return texture_.width();
      }
      return texture_.height();
    }





  private:
    pixglot::square_isometry or_;
    pixglot::gl_texture      texture_;
};

#endif // PHODISPL_FRAME_HPP_INCLUDED
