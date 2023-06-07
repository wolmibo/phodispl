#ifndef WIN_VEC2_HPP_INCLUDED
#define WIN_VEC2_HPP_INCLUDED

#include <compare>



namespace win {

template<typename T>
struct vec2 {
  T x;
  T y;



  auto operator<=>(const vec2<T>&) const = default;
};



template<typename T, typename S>
vec2<T> make_vec2(S x, S y) {
  return vec2<T>{
    .x = static_cast<T>(x),
    .y = static_cast<T>(y)
  };
}

}

#endif // WIN_VEC2_HPP_INCLUDED
