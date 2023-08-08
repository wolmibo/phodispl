#include "win/input-manager-wayland.hpp"
#include "win/modifier.hpp"
#include "win/widget.hpp"
#include "win/window-listener.hpp"
#include "win/window-wayland.hpp"

#include <functional>
#include <utility>

#include <logcerr/log.hpp>

#include <unistd.h>
#include <sys/mman.h>





void win::input_manager_wayland::select_listener(wl_surface* surface) {
  if (auto index = listeners_.find_index(surface)) {
    std::tie(active_listener_, active_widget_) = listeners_.value(*index);
  } else {
    active_listener_ = nullptr;
    active_widget_   = nullptr;
    logcerr::debug("wayland input: active surface not found");
  }
}





void win::input_manager_wayland::register_listener(
    wl_surface*      surface,
    window_listener* listener,
    widget*          widg
) {
  if (auto index = listeners_.find_index(surface)) {
    listeners_.value(*index) = std::make_pair(listener, widg);
  } else {
    listeners_.emplace(surface, listener, widg);
  }
}



void win::input_manager_wayland::unregister_listener(wl_surface* surface) {
  if (auto index = listeners_.find_index(surface)) {
    auto [list, wid] = listeners_.value(*index);

    if (active_listener_ == list) {
      active_listener_ = nullptr;
    }

    if (active_widget_ == wid) {
      active_widget_ = nullptr;
    }

    listeners_.erase(*index);
  }
}





void win::input_manager_wayland::seat(wl_ptr<wl_seat> seat) {
  seat_ = std::move(seat);
  wl_seat_add_listener(seat_.get(), &seat_listener_, this);
}



void win::input_manager_wayland::gestures(wl_ptr<zwp_pointer_gestures_v1> gestures) {
  gestures_ = std::move(gestures);
}





namespace win {
  struct pointer_state {
    bool                         moved   {false};
    vec2<float>                  position{};
    vec2<float>                  delta   {};
    int32_t                      enter   {0};
    chaos_map<uint32_t, int32_t> pressed {};



    void clear() {
      moved    = false;
      delta    = win::vec2<float>{0.f, 0.f};
      enter    = 0;
      pressed.clear();
    }
  };



  struct keyboard_state {
    wl_ptr<xkb_context> context;
    wl_ptr<xkb_state>   state;
    wl_ptr<xkb_keymap>  keymap;


    keyboard_state() :
      context{xkb_context_new(XKB_CONTEXT_NO_FLAGS)}
    {}
  };
}



win::input_manager_wayland::input_manager_wayland() :
  pointer_state_ {std::make_unique<win::pointer_state>()},
  keyboard_state_{std::make_unique<win::keyboard_state>()}
{}





namespace {
  [[nodiscard]] win::vec2<float> make_vec2(wl_fixed_t x, wl_fixed_t y) {
    return win::make_vec2<float>(wl_fixed_to_double(x), wl_fixed_to_double(y));
  }
}





namespace {
  void pointer_enter(
      void*       data,
      wl_pointer* /*pointer*/,
      uint32_t    /*serial*/,
      wl_surface* surface,
      wl_fixed_t  surface_x,
      wl_fixed_t  surface_y
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    self->select_listener(surface);

    self->pointer_state()->position = make_vec2(surface_x, surface_y);
    self->pointer_state()->enter++;
  }



  void pointer_leave(
      void*       data,
      wl_pointer* /*pointer*/,
      uint32_t    /*serial*/,
      wl_surface* /*surface*/
  ) {
    auto* state = static_cast<win::input_manager_wayland*>(data)->pointer_state();

    state->enter--;
  }



  void pointer_motion(
      void*       data,
      wl_pointer* /*pointer*/,
      uint32_t    /*time*/,
      wl_fixed_t  surface_x,
      wl_fixed_t  surface_y
  ) {
    auto* state = static_cast<win::input_manager_wayland*>(data)->pointer_state();

    state->moved    = true;
    state->position = make_vec2(surface_x, surface_y);
  }



  void pointer_button(
      void*       data,
      wl_pointer* /*pointer*/,
      uint32_t    /*serial*/,
      uint32_t    /*time*/,
      uint32_t    button,
      uint32_t    button_state
  ) {
    auto* state = static_cast<win::input_manager_wayland*>(data)->pointer_state();

    state->pressed.find_or_create(button, 0) +=
      (button_state == WL_POINTER_BUTTON_STATE_PRESSED ? 1 : -1);
  }



  void pointer_axis(
      void*       data,
      wl_pointer* /*pointer*/,
      uint32_t    /*time*/,
      uint32_t    axis,
      wl_fixed_t  value
  ) {
    auto* state = static_cast<win::input_manager_wayland*>(data)->pointer_state();

    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
      state->delta.y -= wl_fixed_to_double(value);
    } else {
      state->delta.x += wl_fixed_to_double(value);
    }
  }



  void pointer_axis_source  (void* /*data*/, wl_pointer* /*pointer*/,
      uint32_t /*source*/) {}
  void pointer_axis_stop    (void* /*data*/, wl_pointer* /*pointer*/,
      uint32_t /*time*/, uint32_t /*axis*/) {}
  void pointer_axis_discrete(void* /*data*/, wl_pointer* /*pointer*/,
      uint32_t /*axis*/, int32_t /*discrete*/) {}
  void pointer_axis_value120(void* /*data*/, wl_pointer* /*pointer*/,
      uint32_t /*axis*/, int32_t /*value120*/) {}
  [[maybe_unused]] void pointer_axis_relative_direction(void* /*data*/,
      wl_pointer* /*pointer*/, uint32_t /*axis*/, uint32_t /*direction*/) {}



  void pointer_frame(void* data, wl_pointer* /*pointer*/) {
    auto* self  = static_cast<win::input_manager_wayland*>(data);
    auto* state = self->pointer_state();



    if (state->enter > 0) {
      self->widget_event(&win::widget::pointer_move, state->position);
    }



    if (state->moved) {
      self->widget_event(&win::widget::pointer_move, state->position);
    }

    if (state->delta != win::vec2<float>{0.f, 0.f}) {
      self->widget_event(&win::widget::scroll, state->position, state->delta);
    }



    for (size_t i = 0; i < state->pressed.size(); ++i) {
      if (state->pressed.value(i) > 0) {
        self->widget_event(&win::widget::pointer_press,
            state->position, static_cast<win::mouse_button>(state->pressed.key(i)));
      } else if (state->pressed.value(i) < 0) {
        self->widget_event(&win::widget::pointer_release,
            state->position, static_cast<win::mouse_button>(state->pressed.key(i)));
      }
    }



    if (state->enter < 0) {
      self->widget_event(&win::widget::pointer_leave);
    }



    state->clear();
  }



  constexpr wl_pointer_listener pointer_listener = {
    .enter                   = pointer_enter,
    .leave                   = pointer_leave,
    .motion                  = pointer_motion,
    .button                  = pointer_button,
    .axis                    = pointer_axis,
    .frame                   = pointer_frame,
    .axis_source             = pointer_axis_source,
    .axis_stop               = pointer_axis_stop,
    .axis_discrete           = pointer_axis_discrete,
    .axis_value120           = pointer_axis_value120,
#ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
    .axis_relative_direction = pointer_axis_relative_direction,
#endif
  };
}





namespace {
  void on_key(uint32_t key, uint32_t state, win::input_manager_wayland& manager) {
    uint32_t sym = xkb_state_key_get_one_sym(manager.keyboard_state()->state.get(), key);

    if (sym < 0xff00) {
      sym = xkb_keysym_to_utf32(sym);
    }

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
      manager.event(&win::window_listener::on_key_press, static_cast<win::key>(sym));
    } else {
      manager.event(&win::window_listener::on_key_release, static_cast<win::key>(sym));
    }
  }



  void keyboard_keymap(
      void*        data,
      wl_keyboard* /*keyboard*/,
      uint32_t     format,
      int32_t      fd,
      uint32_t     size
  ) {
    auto* state = static_cast<win::input_manager_wayland*>(data)->keyboard_state();

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
      logcerr::warn("keymap: only xkb_v1 is supported");
      return;
    }

    auto* shm = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (shm == MAP_FAILED) {
      logcerr::warn("unable to access keymap fd");
      return;
    }

    state->keymap.reset(xkb_keymap_new_from_string(
          state->context.get(),
          static_cast<char*>(shm),
          XKB_KEYMAP_FORMAT_TEXT_V1,
          XKB_KEYMAP_COMPILE_NO_FLAGS
    ));

    munmap(shm, size);
    close(fd);

    state->state.reset(xkb_state_new(state->keymap.get()));
  }



  void keyboard_enter(
      void*        data,
      wl_keyboard* /*keyboard*/,
      uint32_t     /*serial*/,
      wl_surface*  surface,
      wl_array*    keys
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    self->select_listener(surface);
    self->event(&win::window_listener::on_key_enter);

    for (auto key: wl_array_to_span<uint32_t>(keys)) {
      on_key(key + 8, WL_KEYBOARD_KEY_STATE_PRESSED, *self);
    }
  }



  void keyboard_leave(
      void*        data,
      wl_keyboard* /*keyboard*/,
      uint32_t     /*serial*/,
      wl_surface*  /*surface*/
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);
    self->event(&win::window_listener::on_key_leave);
  }



  void keyboard_key(
      void*        data,
      wl_keyboard* /*keyboard*/,
      uint32_t     /*serial*/,
      uint32_t     /*time*/,
      uint32_t     key,
      uint32_t     state
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);
    on_key(key + 8, state, *self);
  }



  void keyboard_modifiers(
      void*        data,
      wl_keyboard* /*keyboard*/,
      uint32_t     /*serial*/,
      uint32_t     mods_depressed,
      uint32_t     mods_latched,
      uint32_t     mods_locked,
      uint32_t     group
  ) {
    auto* state = static_cast<win::input_manager_wayland*>(data)->keyboard_state();

    xkb_state_update_mask(state->state.get(),
        mods_depressed, mods_latched, mods_locked, 0, 0, group);
  }



  void keyboard_repeat_info(void* /*data*/, wl_keyboard* /*keyboard*/,
      int32_t /*rate*/, int32_t /*delay*/) {}



  constexpr wl_keyboard_listener keyboard_listener = {
    .keymap      = keyboard_keymap,
    .enter       = keyboard_enter,
    .leave       = keyboard_leave,
    .key         = keyboard_key,
    .modifiers   = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
  };
}





bool win::input_manager_wayland::mod_active(win::modifier mod) const {
  if (!keyboard_state_->state) {
    return false;
  }

  return xkb_state_mod_index_is_active(keyboard_state_->state.get(),
      std::to_underlying(mod), XKB_STATE_MODS_EFFECTIVE) != 0;
}





void win::input_manager_wayland::seat_capability(
    void*    data,
    wl_seat* seat,
    uint32_t capabilities
) {
  auto* self = static_cast<input_manager_wayland*>(data);

  if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) != 0) {
    self->keyboard_.reset(wl_seat_get_keyboard(seat));
    if (!self->keyboard_) {
      logcerr::warn("seat: unable to retrieve keyboard");
    } else {
      wl_keyboard_add_listener(self->keyboard_.get(), &keyboard_listener, data);
    }
  }

  if ((capabilities & WL_SEAT_CAPABILITY_POINTER) != 0) {
    self->pointer_.reset(wl_seat_get_pointer(seat));
    if (!self->pointer_) {
      logcerr::warn("seat: unable to retrieve pointer");
    } else {
      wl_pointer_add_listener(self->pointer_.get(), &pointer_listener, data);
    }

    self->create_gestures();
  }
}



void win::input_manager_wayland::seat_name(
    void*       /*data*/,
    wl_seat*    /*seat*/,
    const char* name
) {
  logcerr::debug("active seat: {}", name);
}








namespace {
  void pinch_begin(
      void*                         data,
      zwp_pointer_gesture_pinch_v1* /*pinch*/,
      uint32_t                      /*serial*/,
      uint32_t                      /*time*/,
      wl_surface*                   surface,
      uint32_t                      /*fingers*/
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    self->select_listener(surface);
    self->event(&win::window_listener::on_pinch_begin, self->pointer_state()->position);
  }



  void pinch_update(
      void*                         data,
      zwp_pointer_gesture_pinch_v1* /*pinch*/,
      uint32_t                      /*time*/,
      wl_fixed_t                    dx,
      wl_fixed_t                    dy,
      wl_fixed_t                    scale,
      wl_fixed_t                    rotation
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    self->event(&win::window_listener::on_pinch_update, make_vec2(dx, dy),
        wl_fixed_to_double(scale), wl_fixed_to_double(rotation));
  }



  void pinch_end(
      void*                         data,
      zwp_pointer_gesture_pinch_v1* /*pinch*/,
      uint32_t                      /*serial*/,
      uint32_t                      /*time*/,
      int32_t                       cancelled
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    if (cancelled == 1) {
      self->event(&win::window_listener::on_pinch_cancel);
    } else {
      self->event(&win::window_listener::on_pinch_finish);
    }
  }



  constexpr zwp_pointer_gesture_pinch_v1_listener pinch_listener = {
    .begin  = pinch_begin,
    .update = pinch_update,
    .end    = pinch_end
  };
}





namespace {
  void swipe_begin(
      void*                         data,
      zwp_pointer_gesture_swipe_v1* /*swipe*/,
      uint32_t                      /*serial*/,
      uint32_t                      /*time*/,
      wl_surface*                   surface,
      uint32_t                      fingers
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    self->select_listener(surface);
    self->event(&win::window_listener::on_swipe_begin,
        self->pointer_state()->position, fingers);
  }



  void swipe_update(
      void*                         data,
      zwp_pointer_gesture_swipe_v1* /*swipe*/,
      uint32_t                      /*time*/,
      wl_fixed_t                    dx,
      wl_fixed_t                    dy
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    self->event(&win::window_listener::on_swipe_update, make_vec2(dx, dy));
  }



  void swipe_end(
      void*                         data,
      zwp_pointer_gesture_swipe_v1* /*swipe*/,
      uint32_t                      /*serial*/,
      uint32_t                      /*time*/,
      int32_t                       cancelled
  ) {
    auto* self = static_cast<win::input_manager_wayland*>(data);

    if (cancelled == 1) {
      self->event(&win::window_listener::on_swipe_cancel);
    } else {
      self->event(&win::window_listener::on_swipe_finish);
    }
  }



  constexpr zwp_pointer_gesture_swipe_v1_listener swipe_listener {
    .begin  = swipe_begin,
    .update = swipe_update,
    .end    = swipe_end
  };
}





void win::input_manager_wayland::create_gestures() {
  if (!gestures_ || !pointer_) {
    return;
  }



  swipe_.reset(zwp_pointer_gestures_v1_get_swipe_gesture(gestures_.get(),
        pointer_.get()));

  if (!swipe_) {
    logcerr::warn("unable to obtain swipe gesture");
  } else {
    zwp_pointer_gesture_swipe_v1_add_listener(swipe_.get(), &swipe_listener, this);
  }



  pinch_.reset(zwp_pointer_gestures_v1_get_pinch_gesture(gestures_.get(),
        pointer_.get()));

  if (!pinch_) {
    logcerr::warn("unable to obtain pinch gesture");
  } else {
    zwp_pointer_gesture_pinch_v1_add_listener(pinch_.get(), &pinch_listener, this);
  }
}


win::input_manager_wayland::~input_manager_wayland() = default;
