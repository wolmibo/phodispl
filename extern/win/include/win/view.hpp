// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_VIEW_HPP_INCLUDED
#define WIN_VIEW_HPP_INCLUDED

#include "win/vec2.hpp"
#include "win/types.hpp"



namespace win {

class view {
  public:
    [[nodiscard]] vec2<float> logical_size()      const { return size_; }
    [[nodiscard]] vec2<float> logical_position()  const { return position_; }

    [[nodiscard]] vec2<float> physical_size()     const { return scale_ * size_;   }
    [[nodiscard]] vec2<float> physical_position() const { return scale_ * position_; }

    [[nodiscard]] float       scale()             const { return scale_; }



    [[nodiscard]] mat4 trafo_mat_logical (vec2<float>, vec2<float>) const;
    [[nodiscard]] mat4 trafo_mat_physical(vec2<float>, vec2<float>) const;





  private:
    vec2<float> size_;
    vec2<float> position_;
    float       scale_;
};

}

#endif // WIN_VIEW_HPP_INCLUDED
