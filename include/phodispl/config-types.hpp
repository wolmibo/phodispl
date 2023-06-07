#ifndef PHODISPL_CONFIG_TYPES_HPP_INCLUDED
#define PHODISPL_CONFIG_TYPES_HPP_INCLUDED

#include "phodispl/animation-interpolation.hpp"

#include <array>



using color = std::array<float, 4>;



enum class scale_filter {
  linear,
  nearest
};


#endif // PHODISPL_CONFIG_TYPES_HPP_INCLUDED
