// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_TYPES_HPP_INCLUDED
#define WIN_TYPES_HPP_INCLUDED

#include <array>

namespace win {
  using color = std::array<float, 4>;

  enum class alpha_mode {
    premultiplied,
    straight
  };
}

#endif // WIN_TYPES_HPP_INCLUDED
