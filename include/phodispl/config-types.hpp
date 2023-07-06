#ifndef PHODISPL_CONFIG_TYPES_HPP_INCLUDED
#define PHODISPL_CONFIG_TYPES_HPP_INCLUDED

#include <gl/base.hpp>

#include <array>



using color = std::array<float, 4>;



enum class scale_filter : GLint {
  linear  = GL_LINEAR,
  nearest = GL_NEAREST
};



enum class listing_mode {
  always    = 0,
  exists    = 1,
  supported = 2,
};


#endif // PHODISPL_CONFIG_TYPES_HPP_INCLUDED
