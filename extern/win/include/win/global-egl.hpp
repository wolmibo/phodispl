#ifndef WIN_GLOBAL_EGL_HPP_INCLUDED
#define WIN_GLOBAL_EGL_HPP_INCLUDED

#include "win/context-wayland.hpp"

#include <wayland-client.h>
#include <wayland-egl.h>
// must be included after wayland-*
#include <EGL/egl.h>



namespace win {

class global_egl {
  public:
    global_egl(wl_display*);

    [[nodiscard]] context_wayland create_context(wl_egl_window*)         const;
    [[nodiscard]] context_wayland create_context(const context_wayland&) const;



  private:
    EGLDisplay display_{EGL_NO_DISPLAY};
    EGLConfig  config_ {nullptr};
};

}

#endif // WIN_GLOBAL_EGL_HPP_INCLUDED
