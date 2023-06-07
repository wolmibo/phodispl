#include "phodispl/box.hpp"

#include <cmath>



mat4 box::to_matrix(float w, float h) const {
  float sx = width  / w;
  float sy = height / h;

  float x_ = x;
  float y_ = y;

  float dx = (w - width ) / 2;
  float dy = (h - height) / 2;

  if (anchor == placement::center) {
    x_ += dx;
    y_ += dy;
  }

  if (align_pixels) {
    x_ = std::floor(x_);
    y_ = std::floor(y_);
  }


  x_ -= dx;
  y_ -= dy;

  float a = sx;
  float d = sy;

  float e = 2*x_/w;
  float f = 2*y_/h;


  {
    using enum pixglot::square_isometry;

    switch (orientation) {
      case flip_x:
      case transpose:
      case rotate_cw:
      case rotate_half:
        a *= -1;
        f *= -1;
        break;
      default:
        break;
    }

    switch (orientation) {
      case transpose:
      case flip_y:
      case rotate_half:
      case rotate_ccw:
        d *= -1;
        e *= -1;
        break;
      default:
        break;
    }
  }

  if (flips_xy(orientation)) {
    return {
      0, d, 0, 0,
      a, 0, 0, 0,
      0, 0, 1, 0,
      f, e, 0, 1
    };
  }

  return {
    a, 0, 0, 0,
    0, d, 0, 0,
    0, 0, 1, 0,
    e, f, 0, 1
  };
}
