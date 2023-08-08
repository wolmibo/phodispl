#ifndef WIN_INPUT_MANAGER_WAYLAND_HPP_INCLUDED
#define WIN_INPUT_MANAGER_WAYLAND_HPP_INCLUDED

#include "win/modifier.hpp"

#include <functional>

#include <chaos-map.hpp>

#include <wayland-client.h>

#include "win/backend-wayland-utils.hpp"



namespace win {

class window_listener;
class widget;

struct pointer_state;
struct keyboard_state;

class input_manager_wayland {
  public:
    input_manager_wayland(const input_manager_wayland&) = delete;
    input_manager_wayland(input_manager_wayland&&)      = delete;
    input_manager_wayland& operator=(const input_manager_wayland&) = delete;
    input_manager_wayland& operator=(input_manager_wayland&&)      = delete;

    ~input_manager_wayland();

    input_manager_wayland();



    void seat(wl_ptr<wl_seat>);
    void gestures(wl_ptr<zwp_pointer_gestures_v1>);

    void register_listener  (wl_surface*, window_listener*, widget*);
    void unregister_listener(wl_surface*);

    [[nodiscard]] wl_seat*                 seat()     const { return seat_.get();     }
    [[nodiscard]] zwp_pointer_gestures_v1* gestures() const { return gestures_.get(); }



    template<typename Fnc, typename... Args>
    void event(Fnc&& fnc, Args&&... args) {
      if (active_listener_ != nullptr) {
        std::invoke(std::forward<Fnc>(fnc),
            active_listener_, std::forward<Args>(args)...);
      }
    }

    template<typename Fnc, typename... Args>
    void widget_event(Fnc&& fnc, Args&&... args) {
      if (active_widget_ != nullptr) {
        std::invoke(std::forward<Fnc>(fnc),
            active_widget_, std::forward<Args>(args)...);
      }
    }

    void select_listener(wl_surface*);


    [[nodiscard]] win::pointer_state*  pointer_state()  { return pointer_state_.get();  }
    [[nodiscard]] win::keyboard_state* keyboard_state() { return keyboard_state_.get(); }


    [[nodiscard]] bool mod_active(modifier) const;



  private:
    wl_ptr<wl_seat>                          seat_;
    wl_ptr<wl_keyboard>                      keyboard_;
    wl_ptr<wl_pointer>                       pointer_;

    wl_ptr<zwp_pointer_gestures_v1>          gestures_;
    wl_ptr<zwp_pointer_gesture_swipe_v1>     swipe_;
    wl_ptr<zwp_pointer_gesture_pinch_v1>     pinch_;

    chaos_map<wl_surface*, std::pair<window_listener*, widget*>>
                                             listeners_;
    window_listener*                         active_listener_{nullptr};
    widget*                                  active_widget_{nullptr};

    std::unique_ptr<win::pointer_state>      pointer_state_;
    std::unique_ptr<win::keyboard_state>     keyboard_state_;



    void create_gestures();



    static void seat_capability(void*, wl_seat*, uint32_t);
    static void seat_name      (void*, wl_seat*, const char*);

    static constexpr wl_seat_listener seat_listener_ {
      .capabilities = seat_capability,
      .name         = seat_name,
    };
};

}

#endif // WIN_INPUT_MANAGER_WAYLAND_HPP_INCLUDED
