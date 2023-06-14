#include "phodispl/movement.hpp"

#include <utility>



namespace {
  template<size_t Size>
  [[nodiscard]] float to_float(std::bitset<Size> bitset, movement::direction dir) {
    return bitset[std::to_underlying(dir)] ? 1.f : 0.f;
  }
}



std::array<float, 2> movement::to_vector() const {
  return {
    to_float(direction_, direction::right) - to_float(direction_, direction::left),
    to_float(direction_, direction::up   ) - to_float(direction_, direction::down)
  };
}
