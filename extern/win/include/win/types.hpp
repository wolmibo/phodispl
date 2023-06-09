// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_TYPES_HPP_INCLUDED
#define WIN_TYPES_HPP_INCLUDED

#include <array>

namespace win {
  using color = std::array<float, 4>;

  using mat4 = std::array<float, 16>;



  void set_uniform_mat4(int, const mat4&);
}

#endif // WIN_TYPES_HPP_INCLUDED
