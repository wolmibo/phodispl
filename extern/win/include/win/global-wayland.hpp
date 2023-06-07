#ifndef WIN_GLOBAL_WAYLAND_HPP_INCLUDED
#define WIN_GLOBAL_WAYLAND_HPP_INCLUDED

#include "win/global-egl.hpp"
#include "win/input-manager-wayland.hpp"



namespace win {

class global_wayland {
  public:
    global_wayland(const global_wayland&) = delete;
    global_wayland(global_wayland&&)      = delete;
    global_wayland& operator=(const global_wayland&) = delete;
    global_wayland& operator=(global_wayland&&)      = delete;

    ~global_wayland() = default;

    global_wayland();

    int  dispatch();
    void roundtrip();



    [[nodiscard]] input_manager_wayland& input_manager() { return input_manager_; }
    [[nodiscard]] const input_manager_wayland& input_manager() const {
      return input_manager_;
    }

    [[nodiscard]] const global_egl& egl() const { return egl_display_; }



    [[nodiscard]] wl_compositor* compositor() const { return compositor_.get(); }
    [[nodiscard]] xdg_wm_base*   wm_base()    const { return wm_base_.get();    }

    [[nodiscard]] zxdg_decoration_manager_v1* decoration_manager() const {
      return decoration_manager_.get();
    }



  private:
    wl_ptr<wl_display>                 display_;
    wl_ptr<wl_registry>                registry_;

    global_egl                         egl_display_;

    wl_ptr<wl_compositor>              compositor_;

    wl_ptr<xdg_wm_base>                wm_base_;
    wl_ptr<zxdg_decoration_manager_v1> decoration_manager_;

    input_manager_wayland              input_manager_;



    static void global(void*, wl_registry*, uint32_t, const char*, uint32_t);
    static void global_remove(void* /*data*/, wl_registry* /*reg*/, uint32_t /*name*/) {};

    static constexpr wl_registry_listener registry_listener_ = {
      .global        = global,
      .global_remove = global_remove
    };



    static void wm_base_ping(void*, xdg_wm_base*, uint32_t);

    static constexpr xdg_wm_base_listener wm_base_listener_ = {
      .ping = wm_base_ping,
    };
};
}

#endif // WIN_GLOBAL_WAYLAND_HPP_INCLUDED
