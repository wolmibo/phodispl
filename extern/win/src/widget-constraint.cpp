#include "win/widget-constraint.hpp"



namespace {
  [[nodiscard]] float extent(win::dimension_constraint inner, float outer, float margin) {
    if (std::holds_alternative<win::dimension_fill_constraint>(inner)) {
      return outer - margin;
    }
    return std::get<float>(inner);
  }



  [[nodiscard]] float shift(
      float                extent,
      float                outer,
      std::optional<float> start,
      std::optional<float> end
  ) {
    if (!start && !end) {
      return 0;
    }

    if (!end) {
      return *start;
    }

    if (!start) {
      return outer - extent - *end;
    }

    return (outer - extent - *start - *end) / 2.f;
  }
}





std::array<float, 4> win::widget_constraint::realize(
    vec2<float> outer_size
) const {
  std::array<float, 4> bounds{0.f};

  bounds[2] = extent(width, outer_size.x,
                     margin.start.value_or(0.f) + margin.end.value_or(0.f));
  bounds[3] = extent(height, outer_size.y,
                     margin.top.value_or(0.f) + margin.bottom.value_or(0.f));

  bounds[0] = shift(bounds[2], outer_size.x, margin.start, margin.end);
  bounds[1] = shift(bounds[3], outer_size.y, margin.top, margin.bottom);

  return bounds;
}
