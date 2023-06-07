#ifndef WIN_BACKEND_WAYLAND_HPP_INCLUDED
#define WIN_BACKEND_WAYLAND_HPP_INCLUDED

#include "win/backend-wayland-utils.hpp"
#include "win/wayland/egl.hpp"
#include "win/wayland/global.hpp"

#include <memory>
#include <optional>

#include <chaos-map.hpp>

#include <wayland-egl.h>



namespace win {

class backend;
class window;

class backend_wayland : public backend, protected wayland::global {
  public:
    backend_wayland(const backend_wayland&) = delete;
    backend_wayland(backend_wayland&&)      = delete;
    backend_wayland& operator=(const backend_wayland&) = delete;
    backend_wayland& operator=(backend_wayland&&)      = delete;

    ~backend_wayland() override = default;

    explicit backend_wayland(window*);



  private:
    wl_ptr<wl_compositor>                compositor_;
    wl_ptr<xdg_wm_base>                  xdg_base_;

    wl_ptr<wl_seat>                      seat_;
    wl_ptr<wl_keyboard>                  keyboard_;
    wl_ptr<wl_pointer>                   pointer_;

    wl_ptr<xkb_state>                    xkb_state_;
    wl_ptr<xkb_context>                  xkb_context_;
    wl_ptr<xkb_keymap>                   xkb_keymap_;

    wl_ptr<wl_surface>                   surface_;
    wl_ptr<xdg_surface>                  xdg_surface_;
    wl_ptr<xdg_toplevel>                 xdg_toplevel_;

    wl_ptr<zxdg_decoration_manager_v1>   zxdg_decoration_manager_;
    wl_ptr<zxdg_toplevel_decoration_v1>  zxdg_toplevel_decoration_;

    wl_ptr<zwp_pointer_gestures_v1>      zwp_pointer_gestures_;
    wl_ptr<zwp_pointer_gesture_swipe_v1> zwp_pointer_gesture_swipe_;
    wl_ptr<zwp_pointer_gesture_pinch_v1> zwp_pointer_gesture_pinch_;

    wl_ptr<wl_callback>                  frame_callback_;

    wl_ptr<wl_egl_window>                egl_window_;

    uint32_t                             width_           = 640;
    uint32_t                             height_          = 480;

    bool                                 closed_          = false;

    bool                                 fullscreen_      = false;

    std::optional<egl_display>           egl_display_;
    std::optional<egl_context>           egl_context_;

    bool                                 frame_requested_ = true;

    struct pointer_state {
      bool                                 moved            = false;
      double                               x                = 0.;
      double                               y                = 0.;
      double                               dx               = 0.;
      double                               dy               = 0.;
      chaos_map<uint32_t, int32_t>         pressed;
      int32_t                              enter            = 0;
    } pointer_state_;

    void commit() {
      if (surface_) {
        wl_surface_commit(surface_.get());
      }
    }




    // Wayland listener defined in "wayland-listener.hpp"

    DEFINE_XDG_WM_BASE_LISTENER

    DEFINE_SEAT_LISTENER
    DEFINE_KEYBOARD_LISTENER
    DEFINE_POINTER_LISTENER

    DEFINE_XDG_SURFACE_LISTENER
    DEFINE_XDG_TOPLEVEL_LISTENER

    DEFINE_CALLBACK_LISTENER

    DEFINE_ZXDG_TOPLEVEL_DECORATION_LISTENER

    DEFINE_ZWP_POINTER_GESTURE_SWIPE_LISTENER
    DEFINE_ZWP_POINTER_GESTURE_PINCH_LISTENER




    void init_surface();
    void init_egl();



    void main_loop() override;
    void close()     override { closed_ = true; }



    void on_key(uint32_t, uint32_t);

    [[nodiscard]] bool mod_active(uint32_t /*modifier*/) const override;



    void title (const std::string& /*title*/) override;
    void app_id(const std::string& /*appid*/) override;



    [[nodiscard]] bool fullscreen() const override { return fullscreen_; }
    void fullscreen(bool /*fs*/) override;



    void on_registry_global(uint32_t /*name*/, std::string_view /*interface*/,
                               uint32_t /*version*/) override;

    [[nodiscard]] std::unique_ptr<gl_context> create_background_context() const override;
};

}


#endif // WIN_BACKEND_WAYLAND_HPP_INCLUDED
