#ifndef PHODISPL_CONFIG_TYPES_HPP_INCLUDED
#define PHODISPL_CONFIG_TYPES_HPP_INCLUDED

#include <gl/base.hpp>

#include <array>



using color = std::array<float, 4>;



enum class scale_filter : GLint {
  linear  = GL_LINEAR,
  nearest = GL_NEAREST
};


#endif // PHODISPL_CONFIG_TYPES_HPP_INCLUDED
