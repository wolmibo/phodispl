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



template<typename T>
[[nodiscard]] vec2<T> operator*(vec2<T> lhs, T scalar) {
  lhs.x *= scalar; lhs.y *= scalar;
  return lhs;
}

template<typename T>
[[nodiscard]] vec2<T> operator*(T scalar, vec2<T> lhs) {
  lhs.x *= scalar; lhs.y *= scalar;
  return lhs;
}

template<typename T>
[[nodiscard]] vec2<T> operator+(vec2<T> lhs, vec2<T> rhs) {
  lhs.x += rhs.x; lhs.y += rhs.y;
  return lhs;
}

template<typename T>
[[nodiscard]] vec2<T> operator-(vec2<T> lhs, vec2<T> rhs) {
  lhs.x -= rhs.x; lhs.y -= rhs.y;
  return lhs;
}



template<typename T>
[[nodiscard]] vec2<T> vec2_mul(vec2<T> lhs, vec2<T> rhs) {
  lhs.x *= rhs.x; lhs.y *= rhs.y;
  return lhs;
}

template<typename T>
[[nodiscard]] vec2<T> vec2_div(vec2<T> lhs, vec2<T> rhs) {
  lhs.x /= rhs.x; lhs.y /= rhs.y;
  return lhs;
}



template<typename T, typename S>
vec2<T> make_vec2(S x, S y) {
  return vec2<T>{
    .x = static_cast<T>(x),
    .y = static_cast<T>(y)
  };
}

}

#endif // WIN_VEC2_HPP_INCLUDED
