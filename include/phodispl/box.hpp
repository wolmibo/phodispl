#ifndef PHODISPL_BOX_HPP_INCLUDED
#define PHODISPL_BOX_HPP_INCLUDED

#include <array>

#include <pixglot/square-isometry.hpp>



using mat4 = std::array<float, 16>;

static constexpr mat4 mat4_unity = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};



enum class placement {
  center,
};



struct box {
  float     width             {0.f};
  float     height            {0.f};

  float     x                 {0.f};
  float     y                 {0.f};

  placement anchor           {placement::center};
  bool      align_pixels     {false};

  pixglot::square_isometry orientation{pixglot::square_isometry::identity};



  [[nodiscard]] mat4 to_matrix(float, float) const;
};

#endif // PHODISPL_BOX_HPP_INCLUDED
