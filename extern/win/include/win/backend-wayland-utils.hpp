#ifndef WIN_BACKEND_WAYLAND_UTILS_HPP_INCLUDED
#define WIN_BACKEND_WAYLAND_UTILS_HPP_INCLUDED

#include <memory>
#include <span>
#include <stdexcept>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-util.h>

#include <xkbcommon/xkbcommon.h>
#include "pointer-gestures-unstable-v1-client-protocol.h"
#include "viewporter-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"



template<typename T>
std::span<T> wl_array_to_span(wl_array *wla) {
  if (wla->size % sizeof(T) != 0) {
    throw std::runtime_error{
        "wl_array does not contain an integer amount of entries"};
  }

  return std::span<T>{static_cast<T*>(wla->data), wla->size / sizeof(T)};
}



template <typename T> struct wayland_deleter {};
template <typename T> struct wayland_interface {};

// NOLINTBEGIN(*macro-usage,*macro-parentheses)
#define DEFINE_WAYLAND_DELETER(x, y)                                           \
  template <> struct wayland_deleter<x> {                                      \
    void operator()(x *ptr) const { x##_##y(ptr); }                            \
  };

#define DEFINE_WAYLAND_INTERFACE(x)                                            \
  template <> struct wayland_interface<x> {                                    \
    static const wl_interface *get() { return &x##_interface; }                \
  };
// NOLINTEND(*macro-usage,*macro-parentheses)



DEFINE_WAYLAND_DELETER(wl_display, disconnect);
DEFINE_WAYLAND_DELETER(wl_registry, destroy);

DEFINE_WAYLAND_DELETER(wl_surface, destroy);
DEFINE_WAYLAND_DELETER(wl_compositor, destroy);
DEFINE_WAYLAND_DELETER(wl_callback, destroy);
DEFINE_WAYLAND_DELETER(wl_seat, destroy);

DEFINE_WAYLAND_DELETER(wl_keyboard, release);
DEFINE_WAYLAND_DELETER(wl_pointer, release);

DEFINE_WAYLAND_INTERFACE(wl_compositor);
DEFINE_WAYLAND_INTERFACE(wl_seat);



#ifdef XDG_WM_BASE_INTERFACE
DEFINE_WAYLAND_DELETER(xdg_wm_base, destroy);
DEFINE_WAYLAND_DELETER(xdg_surface, destroy);
DEFINE_WAYLAND_DELETER(xdg_toplevel, destroy);

DEFINE_WAYLAND_INTERFACE(xdg_wm_base);
#endif



#ifdef ZXDG_DECORATION_MANAGER_V1_INTERFACE
DEFINE_WAYLAND_DELETER(zxdg_decoration_manager_v1, destroy);
DEFINE_WAYLAND_DELETER(zxdg_toplevel_decoration_v1, destroy);

DEFINE_WAYLAND_INTERFACE(zxdg_decoration_manager_v1);
#endif



#ifdef ZWP_POINTER_GESTURES_V1_INTERFACE
DEFINE_WAYLAND_DELETER(zwp_pointer_gestures_v1, destroy);
DEFINE_WAYLAND_DELETER(zwp_pointer_gesture_swipe_v1, destroy);
DEFINE_WAYLAND_DELETER(zwp_pointer_gesture_pinch_v1, destroy);

DEFINE_WAYLAND_INTERFACE(zwp_pointer_gestures_v1);
#endif



#ifdef WP_VIEWPORT_INTERFACE
DEFINE_WAYLAND_DELETER(wp_viewport, destroy);
DEFINE_WAYLAND_DELETER(wp_viewporter, destroy);

DEFINE_WAYLAND_INTERFACE(wp_viewporter);
#endif



#ifdef WAYLAND_EGL_H
DEFINE_WAYLAND_DELETER(wl_egl_window, destroy);
#endif



#ifdef _XKBCOMMON_H_
DEFINE_WAYLAND_DELETER(xkb_context, unref);
DEFINE_WAYLAND_DELETER(xkb_state, unref);
DEFINE_WAYLAND_DELETER(xkb_keymap, unref);
#endif



#undef DEFINE_WAYLAND_INTERFACE
#undef DEFINE_WAYLAND_DELETER

template<typename T> using wl_ptr = std::unique_ptr<T, wayland_deleter<T>>;

#endif // WIN_BACKEND_WAYLAND_UTILS_HPP_INCLUDED
