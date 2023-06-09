// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_VIEW_CONSTRAINT_HPP_INCLUDED
#define WIN_VIEW_CONSTRAINT_HPP_INCLUDED

#include "win/vec2.hpp"

#include <array>



namespace win {

struct view_constraint {
  float left   {-1};
  float right  {-1};
  float top    {-1};
  float bottom {-1};



  [[nodiscard]] std::array<float, 4> realize(vec2<float>, vec2<float>) const;
};

}

#endif // WIN_VIEW_CONSTRAINT_HPP_INCLUDED
