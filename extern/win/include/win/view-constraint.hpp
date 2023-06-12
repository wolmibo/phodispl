// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_VIEW_CONSTRAINT_HPP_INCLUDED
#define WIN_VIEW_CONSTRAINT_HPP_INCLUDED

#include "win/vec2.hpp"

#include <array>
#include <optional>
#include <variant>



namespace win {

struct margin_constraint {
  std::optional<float> start;
  std::optional<float> end;
  std::optional<float> top;
  std::optional<float> bottom;
};



struct dimension_fill_constraint{};

using dimension_constraint = std::variant<dimension_fill_constraint, float>;



struct view_constraint {
  dimension_constraint width;
  dimension_constraint height;

  margin_constraint    margin;

  [[nodiscard]] std::array<float, 4> realize(vec2<float>) const;
};

}

#endif // WIN_VIEW_CONSTRAINT_HPP_INCLUDED
