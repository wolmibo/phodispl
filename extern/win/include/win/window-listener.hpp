#ifndef WIN_WINDOW_LISTENER_HPP_INCLUDED
#define WIN_WINDOW_LISTENER_HPP_INCLUDED

#include "win/mouse-button.hpp"
#include "win/key.hpp"
#include "win/vec2.hpp"

#include <cstdint>



namespace win {

class window_listener {
  public:
    window_listener(const window_listener&) = default;
    window_listener(window_listener&&)      = default;
    window_listener& operator=(const window_listener&) = default;
    window_listener& operator=(window_listener&&)      = default;

    virtual ~window_listener() = default;
    window_listener() = default;



    virtual bool on_update() { return false; }

    virtual void on_rescale(vec2<uint32_t> /*size*/, float /*scale*/) {}

    virtual void on_key_press  (key /*key*/) {}
    virtual void on_key_release(key /*key*/) {}
    virtual void on_key_leave() {}
    virtual void on_key_enter() {}

    virtual void on_scroll(vec2<float> /*position*/, vec2<float> /*direction*/) {}

    virtual void on_pointer_press  (vec2<float> /*position*/, mouse_button /*button*/) {}
    virtual void on_pointer_release(vec2<float> /*position*/, mouse_button /*button*/) {}

    virtual void on_pointer_enter(vec2<float> /*entry_point*/)  {}
    virtual void on_pointer_move (vec2<float> /*new_position*/) {}
    virtual void on_pointer_leave(){}

    virtual void on_swipe_begin (vec2<float> /*position*/, uint32_t /*finger_count*/) {}
    virtual void on_swipe_update(vec2<float> /*position*/) {}
    virtual void on_swipe_cancel() {}
    virtual void on_swipe_finish() {}

    virtual void on_pinch_begin (vec2<float> /*position*/) {}
    virtual void on_pinch_update(vec2<float> /*delta*/, float /*scale*/, float /*rot*/) {}
    virtual void on_pinch_cancel() {}
    virtual void on_pinch_finish() {}



    virtual void on_render_private() {}
};

}

#endif // WIN_WINDOW_LISTENER_HPP_INCLUDED
