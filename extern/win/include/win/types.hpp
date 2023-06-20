// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_TYPES_HPP_INCLUDED
#define WIN_TYPES_HPP_INCLUDED

#include <array>

namespace win {
  using color = std::array<float, 4>;

  using mat4 = std::array<float, 16>;

  static constexpr mat4 mat4_identity {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f
  };



  void set_uniform_mat4(int, const mat4&);
}

#endif // WIN_TYPES_HPP_INCLUDED
