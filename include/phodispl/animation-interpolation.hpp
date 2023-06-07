#ifndef PHODISPL_ANIMATION_INTERPOLATION_HPP_INCLUDED
#define PHODISPL_ANIMATION_INTERPOLATION_HPP_INCLUDED

#include <cmath>
#include <numbers>



enum class animation_interpolation {
  linear,
  immediate,
  sinusoidal,
};



inline float animation_interpolation_eval(animation_interpolation ipol, float t) {
  switch (ipol) {
    case animation_interpolation::linear:
      return t;
    case animation_interpolation::immediate:
      return 1.f;
    case animation_interpolation::sinusoidal:
      return std::sin(t * std::numbers::pi_v<float> * 0.5);
  }
  return 1.f;
}

#endif // PHODISPL_ANIMATION_INTERPOLATION_HPP_INCLUDED
