// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_WIDGET_HPP_INCLUDED
#define WIN_WIDGET_HPP_INCLUDED

#include "win/types.hpp"
#include "win/vec2.hpp"
#include "win/widget-constraint.hpp"

#include <memory>
#include <vector>



namespace win {

class viewport;

class widget {
  public:
    widget(const widget&) = delete;
    widget(widget&&)      = default;
    widget& operator=(const widget&) = delete;
    widget& operator=(widget&&)      = default;

    virtual ~widget() = default;

    widget() = default;



    [[nodiscard]] vec2<float> logical_size()      const { return realized_size_; }
    [[nodiscard]] vec2<float> logical_position()  const { return position_; }

    [[nodiscard]] vec2<float> physical_size()     const { return scale_ * realized_size_; }
    [[nodiscard]] vec2<float> physical_position() const { return scale_ * position_; }

    [[nodiscard]] float       scale()             const { return scale_; }





    void invalidate()                { invalid_ = true; }
    void invalidate(bool invalidate) { if (invalidate) { invalid_ = true; } }

    [[nodiscard]] bool invalid() const;





    void add_child(widget*, widget_constraint);





  protected:
    void render(const viewport&);
    void update();

    virtual void on_render() {}
    virtual void on_update() {}

    void compute_layout(vec2<float>, vec2<float>, float);



    [[nodiscard]] mat4 trafo_mat_logical (vec2<float>, vec2<float>) const;
    [[nodiscard]] mat4 trafo_mat_physical(vec2<float>, vec2<float>) const;

    [[nodiscard]] const win::viewport& viewport() const;




  private:
    vec2<float>             realized_size_{ 0.f,  0.f};
    vec2<float>             position_     { 0.f,  0.f};
    float                   scale_        {1.f};

    bool                    invalid_      {true};

    std::vector<std::pair<widget*, widget_constraint>>
                            children_;
    const win::viewport*    root_ptr_{nullptr};
};

}

#endif // WIN_WIDGET_HPP_INCLUDED
