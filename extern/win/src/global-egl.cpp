#include "win/global-egl.hpp"

#include "win/context-wayland.hpp"

#include <array>
#include <stdexcept>



namespace {
  constexpr std::array<EGLint, 6> context = {
    EGL_CONTEXT_MAJOR_VERSION, 3,
    EGL_CONTEXT_MINOR_VERSION, 2,
    EGL_NONE,                  EGL_NONE,
  };

  constexpr std::array<EGLint, 14> attributes = {
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_ALPHA_SIZE,      8,
    EGL_NONE,            EGL_NONE,
  };



  [[nodiscard]] EGLDisplay create_display(wl_display* native) {
    EGLDisplay display = eglGetDisplay(native);

    if (display == EGL_NO_DISPLAY) {
      throw std::runtime_error{"unable to obtain egl display"};
    }

    if (eglInitialize(display, nullptr, nullptr) == EGL_FALSE) {
      throw std::runtime_error{"unable to initialize egl"};
    }

    return display;
  }



  [[nodiscard]] EGLConfig create_config(EGLDisplay display) {
    EGLint count{0};
    if (eglGetConfigs(display, nullptr, 0, &count) == EGL_FALSE || count == 0) {
      throw std::runtime_error{"unable to find egl configuration"};
    }

    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
      throw std::runtime_error{"unable to bind egl api"};
    }

    EGLConfig config{nullptr};
    if (eglChooseConfig(display, attributes.data(), &config, 1, &count) == EGL_FALSE ||
        count != 1 || config == nullptr) {
      throw std::runtime_error{"unable to choose egl configuration"};
    }

    return config;
  }
}



win::global_egl::global_egl(wl_display* native) :
  display_{create_display(native)},
  config_ {create_config(display_)}
{}




namespace {
  [[nodiscard]] EGLContext create_context(
      EGLDisplay display,
      EGLConfig  config,
      EGLContext shared
  ) {
    EGLContext ctx = eglCreateContext(display, config, shared, context.data());
    if (ctx == EGL_NO_CONTEXT) {
      throw std::runtime_error{"unable to create context"};
    }
    return ctx;
  }
}


win::context_wayland win::global_egl::create_context(wl_egl_window* native) const {
  EGLSurface surface = eglCreateWindowSurface(display_, config_, native, nullptr);

  if (surface == EGL_NO_SURFACE) {
    throw std::runtime_error{"unable to create egl surface"};
  }

  return context_wayland{
      ::create_context(display_, config_, EGL_NO_CONTEXT),
      surface,
      display_
  };
}



win::context_wayland win::global_egl::create_context(const context_wayland& ctx) const {
  return context_wayland{
      ::create_context(display_, config_, ctx.get()),
      EGL_NO_SURFACE,
      display_
  };
}

