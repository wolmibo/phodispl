#ifndef WIN_WINDOW_WAYLAND_HPP_INCLUDED
#define WIN_WINDOW_WAYLAND_HPP_INCLUDED

#include "win/context-wayland.hpp"
#include "win/global-wayland.hpp"
#include "win/window-native.hpp"



namespace win {

class window_wayland : public window_native {
  public:
    window_wayland(const window_wayland&) = delete;
    window_wayland(window_wayland&&)      = delete;
    window_wayland& operator=(const window_wayland&) = delete;
    window_wayland& operator=(window_wayland&&)      = delete;

    ~window_wayland() override = default;

    explicit window_wayland(const std::string&);



  private:
    bool                                should_close_{false};
    vec2<uint32_t>                      size_{100, 100};

    global_wayland                      wayland_;

    wl_ptr<wl_surface>                  surface_;
    wl_ptr<xdg_surface>                 xdg_surface_;
    wl_ptr<xdg_toplevel>                toplevel_;
    wl_ptr<zxdg_toplevel_decoration_v1> decoration_;

    wl_ptr<wl_egl_window>               egl_window_;
    context_wayland                     context_;

    wl_ptr<wl_callback>                 callback_;
    bool                                frame_requested_{false};




    [[nodiscard]] win::backend backend() const override { return win::backend::wayland; }

    void title(const std::string& /*title*/) override;
    void close() override;
    void run()   override;

    void on_new_listener() override;

    [[nodiscard]] bool    mod_active(modifier /*mod*/) const override;
    [[nodiscard]] context share_context()              const override;





    static void xdg_surface_configure(void*, xdg_surface*, uint32_t);

    static constexpr xdg_surface_listener xdg_surface_listener_ {
      .configure = xdg_surface_configure,
    };



    static void toplevel_configure(void*, xdg_toplevel*, int32_t, int32_t, wl_array*);
    static void toplevel_close(void*, xdg_toplevel*);
    static void toplevel_configure_bounds(void* /*data*/, xdg_toplevel* /*tl*/,
        int32_t /*width*/, int32_t /*height*/) {}
    static void toplevel_wm_capabilities(void* /*data*/, xdg_toplevel* /*tl*/,
        wl_array* /*capabilities*/) {}

    static constexpr xdg_toplevel_listener toplevel_listener_ {
      .configure        = toplevel_configure,
      .close            = toplevel_close,
      .configure_bounds = toplevel_configure_bounds,
      .wm_capabilities  = toplevel_wm_capabilities,
    };



    static void callback_done(void*, wl_callback*, uint32_t);

    static constexpr wl_callback_listener callback_listener_ {
      .done = callback_done,
    };



    static void decoration_configure(void*, zxdg_toplevel_decoration_v1*, uint32_t);

    static constexpr zxdg_toplevel_decoration_v1_listener decoration_listener_ {
      .configure = decoration_configure,
    };
};
}

#endif // WIN_WINDOW_WAYLAND_HPP_INCLUDED
