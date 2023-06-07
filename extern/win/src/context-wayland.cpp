#include "win/context-wayland.hpp"

#include <utility>

#include <EGL/egl.h>

#include <logcerr/log.hpp>



win::context_wayland::~context_wayland() {
  destroy();
}





win::context_wayland& win::context_wayland::operator=(context_wayland&& rhs) noexcept {
  destroy();

  display_ = std::exchange(rhs.display_, EGL_NO_DISPLAY);
  surface_ = std::exchange(rhs.surface_, EGL_NO_SURFACE);
  context_ = std::exchange(rhs.context_, EGL_NO_CONTEXT);

  return *this;
}





win::context_wayland::context_wayland(context_wayland&& rhs) noexcept :
  display_{std::exchange(rhs.display_, EGL_NO_DISPLAY)},
  surface_{std::exchange(rhs.surface_, EGL_NO_SURFACE)},
  context_{std::exchange(rhs.context_, EGL_NO_CONTEXT)}
{}





void win::context_wayland::destroy() {
  try {
    context_wayland::release();
  } catch (std::exception& ex) {
    logcerr::error(ex.what());
  } catch (...) {
    logcerr::error("unknown exception while releasing egl context");
  }



  if (context_ != EGL_NO_CONTEXT) {
    if (eglDestroyContext(display_, context_) == EGL_FALSE) {
      logcerr::error("unable to destroy egl context");
    }
  }

  if (surface_ != EGL_NO_SURFACE) {
    if (eglDestroySurface(display_, surface_) == EGL_FALSE) {
      logcerr::error("unable to destroy egl surface");
    }
  }
}





void win::context_wayland::bind() const {
  if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_FALSE) {
    throw std::runtime_error{"unable to make egl context current"};
  }
}



void win::context_wayland::release() const {
  if (context_ == EGL_NO_CONTEXT || eglGetCurrentContext() != context_) {
    return;
  }

  if (eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)
      == EGL_FALSE) {
    throw std::runtime_error{"unable to clear current context"};
  }
}




void win::context_wayland::swap_buffers() const {
  if (eglSwapBuffers(display_, surface_) == EGL_FALSE) {
    throw std::runtime_error{"unable to swap buffers"};
  }
}





void win::context_wayland::swap_interval(int interval) const {
  if (eglSwapInterval(display_, interval) == EGL_FALSE) {
    throw std::runtime_error{"unable to set egl swap interval to "
      + std::to_string(interval)};
  }
}
