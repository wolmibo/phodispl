#include <gl/base.hpp>

#include "win/window-glfw.hpp"
#include "win/window-listener.hpp"
#include "win/window-native.hpp"
#include "win/window-wayland.hpp"
#include "win-config.h"

#include <logcerr/log.hpp>

#include <unistd.h>



std::string_view win::to_string(backend b) {
  switch (b) {
    case backend::none:    return "none";
    case backend::glfw:    return "glfw";
    case backend::wayland: return "wayland";
  }
  return "<unknown>";
}




namespace {
  [[nodiscard]] win::backend backend_from_string(std::string_view str) {
    if (str == "none")    { return win::backend::none;    }
    if (str == "glfw")    { return win::backend::glfw;    }
    if (str == "wayland") { return win::backend::wayland; }

    throw std::runtime_error{"unknown backend: " + std::string{str}};
  }



  [[nodiscard]] std::optional<std::string_view> get_env(const char* var) {
    if (const char* res = getenv(var); res != nullptr && *res != 0) {
      return res;
    }
    return {};
  }
}




win::backend win::select_backend() {
  if (auto var = get_env("WIN_BACKEND")) {
    return backend_from_string(*var);
  }

#ifdef WIN_WITH_BACKEND_WAYLAND
  if (get_env("WAYLAND_DISPLAY")) {
#ifdef WIN_WITH_BACKEND_GLFW
    if (get_env("XDG_SESSION_DESKTOP") != "gnome")
#endif
    {
      return backend::wayland;
    }
  }
#endif

#ifdef WIN_WITH_BACKEND_GLFW
  return backend::glfw;
#endif

  return backend::none;
}





std::unique_ptr<win::window_native> win::window_native::create(
    win::backend       back,
    const std::string& app_id
) {
  switch (back) {
    case backend::glfw:    return std::make_unique<window_glfw>   (app_id);
    case backend::wayland: return std::make_unique<window_wayland>(app_id);
    case backend::none:    return std::make_unique<window_native> ();

    default:
      throw std::runtime_error{"unimplemented backend: "
        + std::string{to_string(back)}
        + " (" + std::to_string(std::to_underlying(back)) + ")"};
  }
}





void win::window_native::rescale(vec2<uint32_t> size, float scale) {
  damage(size_ != size || scale_ != scale);
  size_  = size;
  scale_ = scale;
  listener()->on_resize_private(make_vec2<float>(size.x, size.y), scale);
  listener()->on_rescale(size, scale);
}



bool win::window_native::update() {
  auto value1 = take_damage();
  auto value2 = listener()->on_update();

  return value1 || value2;
}



void win::window_native::listener(window_listener* listener) {
  listener_ = listener;
  on_new_listener();
}
