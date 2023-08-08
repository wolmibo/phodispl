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



    virtual void on_rescale(vec2<uint32_t> /*size*/, float /*scale*/) {}

    virtual void on_key_press  (key /*key*/) {}
    virtual void on_key_release(key /*key*/) {}
    virtual void on_key_leave() {}
    virtual void on_key_enter() {}

    virtual void on_swipe_begin (vec2<float> /*position*/, uint32_t /*finger_count*/) {}
    virtual void on_swipe_update(vec2<float> /*position*/) {}
    virtual void on_swipe_cancel() {}
    virtual void on_swipe_finish() {}

    virtual void on_pinch_begin (vec2<float> /*position*/) {}
    virtual void on_pinch_update(vec2<float> /*delta*/, float /*scale*/, float /*rot*/) {}
    virtual void on_pinch_cancel() {}
    virtual void on_pinch_finish() {}



    virtual void on_resize_private(vec2<float> /*size*/, float /*scale*/) {}
    virtual void on_update_private() {}
};

}

#endif // WIN_WINDOW_LISTENER_HPP_INCLUDED
