// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_VIEW_HPP_INCLUDED
#define WIN_VIEW_HPP_INCLUDED

#include "win/vec2.hpp"
#include "win/view-constraint.hpp"
#include "win/types.hpp"

#include <memory>
#include <vector>



namespace win {

class view {
  public:
    view(const view&) = delete;
    view(view&&)      = delete;
    view& operator=(const view&) = delete;
    view& operator=(view&&)      = delete;

    virtual ~view() = default;

    view() = default;
    view(vec2<float> size) : size_{size} {}



    [[nodiscard]] vec2<float> size()              const { return size_; }

    [[nodiscard]] vec2<float> logical_size()      const { return realized_size_; }
    [[nodiscard]] vec2<float> logical_position()  const { return position_; }

    [[nodiscard]] vec2<float> physical_size()     const { return scale_ * realized_size_; }
    [[nodiscard]] vec2<float> physical_position() const { return scale_ * position_; }

    [[nodiscard]] float       scale()             const { return scale_; }



    [[nodiscard]] mat4 trafo_mat_logical (vec2<float>, vec2<float>) const;
    [[nodiscard]] mat4 trafo_mat_physical(vec2<float>, vec2<float>) const;



    void invalidate() { invalid_ = true; }
    [[nodiscard]] bool invalid() const;





    void add_child(std::shared_ptr<view>, view_constraint);





  protected:
    void render();
    virtual void on_render() {}

    void compute_layout(vec2<float>, vec2<float>, float);





  private:
    vec2<float>       size_         {-1.f, -1.f};
    vec2<float>       realized_size_{ 0.f,  0.f};
    vec2<float>       position_     { 0.f,  0.f};
    float             scale_        {1.f};

    bool              invalid_      {true};

    std::vector<std::pair<std::shared_ptr<view>, view_constraint>>
                      children_;
};

}

#endif // WIN_VIEW_HPP_INCLUDED
