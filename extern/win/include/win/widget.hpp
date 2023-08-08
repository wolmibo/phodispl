// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_WIDGET_HPP_INCLUDED
#define WIN_WIDGET_HPP_INCLUDED

#include "win/mouse-button.hpp"
#include "win/types.hpp"
#include "win/vec2.hpp"
#include "win/widget-constraint.hpp"

#include <functional>
#include <memory>
#include <vector>



namespace win {

class viewport;

class widget {
  friend class viewport;

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



    void invalidate_layout()         { invalid_layout_ = true; }





    void add_child(widget*, widget_constraint);



    void pointer_press  (vec2<float>, mouse_button);
    void pointer_release(vec2<float>, mouse_button);

    void pointer_move (vec2<float>);
    void pointer_leave();

    void scroll(vec2<float>, vec2<float>);

    void register_update_function(std::move_only_function<void(void)>);





  protected:
    void render();
    void update();

    virtual void on_render() {}
    virtual void on_update() {}
    virtual void on_layout(vec2<std::optional<float>>& /*size*/) {}

    virtual void on_pointer_press  (vec2<float> /*position*/, mouse_button /*button*/) {}
    virtual void on_pointer_release(vec2<float> /*position*/, mouse_button /*button*/) {}

    virtual void on_pointer_enter(vec2<float> /*entry_point*/)  {}
    virtual void on_pointer_move (vec2<float> /*new_position*/) {}
    virtual void on_pointer_leave(){}

    virtual void on_scroll(vec2<float> /*position*/, vec2<float> /*direction*/) {}

    void compute_layout(vec2<float>, vec2<float>, float);



    [[nodiscard]] mat4 trafo_mat_logical (vec2<float>, vec2<float>) const;
    [[nodiscard]] mat4 trafo_mat_physical(vec2<float>, vec2<float>) const;

    //NOLINTNEXTLINE(*-use-nodiscard)
    vec2<float> draw_string(vec2<float>, std::u32string_view, uint32_t, color) const;

    [[nodiscard]] const win::viewport& viewport() const;




  private:
    vec2<float>             realized_size_ { 0.f,  0.f};
    vec2<float>             position_      { 0.f,  0.f};
    float                   scale_         {1.f};

    bool                    invalid_       {true};
    bool                    invalid_layout_{false};

    std::vector<std::pair<widget*, widget_constraint>>
                            children_;
    const win::viewport*    root_ptr_{nullptr};

    bool                    mouse_entered_{false};

    std::vector<std::move_only_function<void(void)>>
                            update_functions_;

    void viewport(const win::viewport&);



    void compute_child_layout(widget*, widget_constraint);
};

}

#endif // WIN_WIDGET_HPP_INCLUDED
