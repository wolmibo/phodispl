#ifndef WIN_CONTEXT_WAYLAND_HPP_INCLUDED
#define WIN_CONTEXT_WAYLAND_HPP_INCLUDED

#include "win/context-native.hpp"

#include <wayland-client.h>
#include <wayland-egl.h>
// must be included after wayland-*
#include <EGL/egl.h>



namespace win {

class context_wayland : public context_native {
  public:
    context_wayland(const context_wayland&) = delete;
    context_wayland(context_wayland&&) noexcept;
    context_wayland& operator=(const context_wayland&) = delete;
    context_wayland& operator=(context_wayland&&) noexcept;

    ~context_wayland() override;

    context_wayland(EGLContext context, EGLSurface surface, EGLDisplay display) :
      display_{display}, surface_{surface}, context_{context}
    {}



    void bind()    const override;
    void release() const override;

    void swap_buffers() const;
    void swap_interval(int) const;

    [[nodiscard]] EGLContext get() const { return context_; }



  private:
    EGLDisplay display_{EGL_NO_DISPLAY};
    EGLSurface surface_{EGL_NO_SURFACE};
    EGLContext context_{EGL_NO_CONTEXT};

    void destroy();
};
}

#endif // WIN_CONTEXT_WAYLAND_HPP_INCLUDED
