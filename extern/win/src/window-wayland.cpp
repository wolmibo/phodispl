#include "fractional-scale-v1-client-protocol.h"

#include "win/application.hpp"
#include "win/context.hpp"
#include "win/global-wayland.hpp"
#include "win/window-wayland.hpp"

#include <logcerr/log.hpp>



namespace {
  template<typename T>
  [[nodiscard]] wl_ptr<T> create_assert(T* ptr, const char* assert) {
    if (ptr == nullptr) {
      throw std::runtime_error{assert};
    }
    return wl_ptr<T>{ptr};
  }



  [[nodiscard]] wl_ptr<xdg_toplevel> create_xdg_toplevel(
      xdg_surface*       surface,
      const std::string& app_id
  ) {
    wl_ptr<xdg_toplevel> toplevel = create_assert(xdg_surface_get_toplevel(surface),
        "unable to create xdg toplevel");

    xdg_toplevel_set_app_id(toplevel.get(), app_id.c_str());

    return toplevel;
  }



  [[nodiscard]] wl_ptr<zxdg_toplevel_decoration_v1> create_decoration(
      zxdg_decoration_manager_v1* manager,
      xdg_toplevel*               toplevel
  ) {
    if (manager == nullptr) {
      return {};
    }

    wl_ptr<zxdg_toplevel_decoration_v1> decoration{
      zxdg_decoration_manager_v1_get_toplevel_decoration(manager, toplevel)};

    if (!decoration) {
      logcerr::warn("unable to obtain toplevel decoration");
    }

    return decoration;
  }



  [[nodiscard]] wl_ptr<wp_viewport> create_viewport(
      wp_viewporter* viewporter,
      wl_surface*    surface
  ) {
    if (viewporter == nullptr) {
      return {};
    }

    wl_ptr<wp_viewport> viewport{wp_viewporter_get_viewport(viewporter, surface)};

    if (!viewport) {
      logcerr::warn("unable to obtain viewport");
    }

    return viewport;
  }



  [[nodiscard]] wl_ptr<wp_fractional_scale_v1> create_wp_scale(
      wp_fractional_scale_manager_v1* scale_manager,
      wl_surface*                     surface
  ) {
    if (scale_manager == nullptr) {
      return {};
    }

    wl_ptr<wp_fractional_scale_v1> scale{
      wp_fractional_scale_manager_v1_get_fractional_scale(scale_manager, surface)};

    if (!scale) {
      logcerr::warn("unable to obtain fractional scale");
    }

    return scale;
  }
}




win::window_wayland::window_wayland(const std::string& app_id) :
  surface_    {create_assert(wl_compositor_create_surface(wayland_.compositor()),
                "unable to create wayland surface")},
  xdg_surface_{create_assert(xdg_wm_base_get_xdg_surface(wayland_.wm_base(),
                surface_.get()),
                "unable to create xdg surface")},
  toplevel_   {create_xdg_toplevel(xdg_surface_.get(), app_id)},
  decoration_ {create_decoration(wayland_.decoration_manager(), toplevel_.get())},
  viewport_   {create_viewport(wayland_.viewporter(), surface_.get())},
  wp_scale_   {create_wp_scale(wayland_.scale_manager(), surface_.get())},
  egl_window_ {create_assert(wl_egl_window_create(surface_.get(), size_.x, size_.y),
                "unable to create egl window")},
  context_    {wayland_.egl().create_context(egl_window_.get())},

  callback_   {create_assert(wl_surface_frame(surface_.get()),
                "unable to get frame callback")}
{
  context_.bind();

  xdg_surface_add_listener(xdg_surface_.get(), &xdg_surface_listener_, this);
  xdg_toplevel_add_listener(toplevel_.get(), &toplevel_listener_, this);
  wl_callback_add_listener(callback_.get(), &callback_listener_, this);

  if (decoration_) {
    zxdg_toplevel_decoration_v1_set_mode(decoration_.get(),
        ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    zxdg_toplevel_decoration_v1_add_listener(decoration_.get(),
        &decoration_listener_, this);
  }

  if (wp_scale_) {
    wp_fractional_scale_v1_add_listener(wp_scale_.get(), &wp_scale_listener_, this);
  }

  wl_surface_commit(surface_.get());

  wayland_.dispatch();
  wayland_.dispatch();

  context_.swap_buffers();
}




void win::window_wayland::on_new_parent() {
  wayland_.input_manager().register_listener(surface_.get(), parent(), parent());
}





void win::window_wayland::title(const std::string& title) {
  xdg_toplevel_set_title(toplevel_.get(), title.c_str());
}



void win::window_wayland::close() {
  should_close_ = true;
}



void win::window_wayland::run() {
  context_.swap_interval(0);

  while (!should_close_ && wayland_.dispatch() != -1) {
    if (frame_requested_ && update()) {
      frame_requested_ = false;
      parent()->render();
      context_.swap_buffers();
    }
  }
}



bool win::window_wayland::mod_active(modifier mod) const {
  return wayland_.input_manager().mod_active(mod);
}



win::context win::window_wayland::share_context() const {
  return win::context{std::make_unique<win::context_wayland>(
      wayland_.egl().create_context(context_))};
}





void win::window_wayland::callback_done(
    void*        data,
    wl_callback* /*callback*/,
    uint32_t     /*time*/)
{
  auto* self = static_cast<window_wayland*>(data);

  self->frame_requested_ = true;

  self->callback_.reset(wl_surface_frame(self->surface_.get()));
  if (!self->callback_) {
    throw std::runtime_error{"unable to obtain frame callback"};
  }

  wl_callback_add_listener(self->callback_.get(), &callback_listener_, self);

  wl_surface_commit(self->surface_.get());
}





void win::window_wayland::xdg_surface_configure(
    void*        /*data*/,
    xdg_surface* surface,
    uint32_t     serial
) {
  xdg_surface_ack_configure(surface, serial);
}





void win::window_wayland::toplevel_configure(
    void*         data,
    xdg_toplevel* /*toplevel*/,
    int32_t       width,
    int32_t       height,
    wl_array*     /*states*/
) {
  if (width <= 0 || height <= 0) {
    return;
  }

  auto* self = static_cast<window_wayland*>(data);

  auto new_size = make_vec2<uint32_t>(width, height);
  if (new_size == self->size_) {
    return;
  }

  self->size_ = new_size;

  self->update_viewport();

  wl_surface_commit(self->surface_.get());
}





void win::window_wayland::toplevel_close(
    void*         data,
    xdg_toplevel* /*toplevel*/
) {
  static_cast<window_wayland*>(data)->close();
}





void win::window_wayland::decoration_configure(
    void*                        /*data*/,
    zxdg_toplevel_decoration_v1* /*tld*/,
    uint32_t                     mode
) {
  switch (mode) {
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
      logcerr::warn("compositor requested client-side decorations: not (yet?) supported");
      break;
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
      break;
    default:
      logcerr::warn("compositor requested unknown window decoration mode");
  }
}





void win::window_wayland::update_viewport() {
  wl_egl_window_resize(egl_window_.get(), size_.x * scale_, size_.y * scale_, 0, 0);
  rescale(size_, scale_);


  if (viewport_) {
    wp_viewport_set_destination(viewport_.get(), size_.x, size_.y);

    wp_viewport_set_source(viewport_.get(),
        wl_fixed_from_int(0),
        wl_fixed_from_int(0),
        wl_fixed_from_int(size_.x * scale_),
        wl_fixed_from_int(size_.y * scale_));
  }
}





void win::window_wayland::preferred_scale(
    void*                   data,
    wp_fractional_scale_v1* /*fractional_scale*/,
    uint32_t                scale
) {
  if (scale == 0) {
    return;
  }

  auto* self = static_cast<window_wayland*>(data);

  auto s = static_cast<float>(scale) / 120.f;

  logcerr::verbose("viewport scale: {:.5}", s);

  if (s == self->scale_) {
    return;
  }

  self->scale_ = s;
  self->update_viewport();
}





void win::window_wayland::min_size(vec2<uint32_t> size) {
  xdg_toplevel_set_min_size(toplevel_.get(), size.x, size.y);
}
