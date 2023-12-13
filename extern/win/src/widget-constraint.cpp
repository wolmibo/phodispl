#include "win/widget-constraint.hpp"



namespace {
  [[nodiscard]] std::optional<float> extent(
      win::dimension_constraint inner,
      float                     outer,
      float                     margin
  ) {
    if (std::holds_alternative<win::dimension_fill_constraint>(inner)) {
      return outer - margin;
    }

    if (std::holds_alternative<win::dimension_compute_constraint>(inner)) {
      return {};
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

    return (outer - extent - *end - *start) / 2.f + *end;
  }
}





std::array<float, 4> win::widget_constraint::realize(
    vec2<float> outer_size
) const {
  std::array<float, 4> bounds{0.f};

  auto size = realize_size(outer_size);

  bounds[2] = size.x().value_or(0.f);
  bounds[3] = size.y().value_or(0.f);

  bounds[0] = shift(bounds[2], outer_size.x(), margin.start, margin.end);
  bounds[1] = shift(bounds[3], outer_size.y(), margin.top, margin.bottom);

  return bounds;
}





vec2<std::optional<float>> win::widget_constraint::realize_size(
    vec2<float> outer_size
) const {
  return {
    extent(width,  outer_size.x(), margin.start.value_or(0.f) + margin.end.value_or(0.f)),
    extent(height, outer_size.y(), margin.top.value_or(0.f) + margin.bottom.value_or(0.f))
  };
}
