#include "win/global-wayland.hpp"

#include <logcerr/log.hpp>



namespace {
  [[noreturn]] void wl_error(const char* fmt, va_list argp) {
    std::string buffer(1024, '\0');
    vsnprintf(buffer.data(), buffer.size(), fmt, argp);
    throw std::runtime_error{"wayland error: " + buffer};
  }



  [[nodiscard]] wl_ptr<wl_display> connect_wayland() {
    wl_log_set_handler_client(wl_error);

    wl_ptr<wl_display> display{wl_display_connect(nullptr)};

    if (!display) {
      throw std::runtime_error{"unable to connect to wayland"};
    }

    return display;
  }



  [[nodiscard]] wl_ptr<wl_registry> get_registry(wl_display* display) {
    wl_ptr<wl_registry> registry{wl_display_get_registry(display)};

    if (!registry) {
      throw std::runtime_error{"unable to obtain registry"};
    }

    return registry;
  }
}



win::global_wayland::global_wayland() :
  display_ {connect_wayland()},
  registry_{get_registry(display_.get())},

  egl_display_{display_.get()}
{
  wl_registry_add_listener(registry_.get(), &registry_listener_, this);

  dispatch();
  roundtrip();

  if (!compositor_) {
    throw std::runtime_error{"unable to find wl_compositor"};
  }

  if (!wm_base_) {
    throw std::runtime_error{"unable to find xdg_wm_base"};
  }



  if (input_manager_.seat() == nullptr) {
    logcerr::warn("unable to find wl_seat: input will not work");
  }

  if (input_manager_.gestures() == nullptr) {
    logcerr::debug("unable to find zwp_pointer_gestures_v1");
  }



  if (!decoration_manager_) {
    logcerr::debug("unable to find zxdg_decoration_manager_v1");
  }

  if (!viewporter_) {
    logcerr::debug("unable to find wp_viewporter");
  }
}



void win::global_wayland::roundtrip() {
  wl_display_roundtrip(display_.get());
}



int win::global_wayland::dispatch() {
  return wl_display_dispatch(display_.get());
}



namespace {
  template<typename T>
  [[nodiscard]] wl_ptr<T> bind(wl_registry* registry, uint32_t name, uint32_t version) {
    wl_ptr<T> v{static_cast<T*>(
        wl_registry_bind(registry, name, wayland_interface<T>::get(), version))};

    if (!v) {
      throw std::runtime_error{"unable to bind interface " + std::to_string(name)
        + " as " + std::string{wayland_interface<T>::get()->name} + " (version "
        + std::to_string(version) + ")"};
    }
    return v;
  }
}



void win::global_wayland::global(
    void*        data,
    wl_registry* registry,
    uint32_t     name,
    const char*  iface,
    uint32_t     /*version*/
) {
  std::string_view interface{iface};
  auto* self = static_cast<global_wayland*>(data);

  if (interface == wl_compositor_interface.name) {
    self->compositor_ = bind<wl_compositor>(registry, name, 1);

  } else if (interface == xdg_wm_base_interface.name) {
    self->wm_base_ = bind<xdg_wm_base>(registry, name, 1);
    xdg_wm_base_add_listener(self->wm_base_.get(), &wm_base_listener_, data);

  } else if (interface == wl_seat_interface.name) {
    self->input_manager_.seat(bind<wl_seat>(registry, name, 7));

  } else if (interface == zxdg_decoration_manager_v1_interface.name) {
    self->decoration_manager_ = bind<zxdg_decoration_manager_v1>(registry, name, 1);

  } else if (interface == zwp_pointer_gestures_v1_interface.name) {
    self->input_manager_.gestures(bind<zwp_pointer_gestures_v1>(registry, name, 3));

  } else if (interface == wp_viewporter_interface.name) {
    self->viewporter_  = bind<wp_viewporter>(registry, name, 1);
  }
}





void win::global_wayland::wm_base_ping(
    void*        /*data*/,
    xdg_wm_base* wm_base,
    uint32_t     serial
) {
  xdg_wm_base_pong(wm_base, serial);
}
