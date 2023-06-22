#include <gl/base.hpp>

#include "win/application.hpp"
#include "win/window-glfw.hpp"





win::application::application(application&& rhs) noexcept :
  native_ {std::move(rhs.native_)}
{
  native_->parent(this);
}



win::application& win::application::operator=(application&& rhs) noexcept {
  native_->parent(nullptr);
  native_ = std::move(rhs.native_);
  native_->parent(this);

  return *this;
}





win::application::application(const std::string& app_id) :
  native_{window_native::create(win::select_backend(), app_id)}
{
  native_->parent(this);
}





void win::application::on_resize_private(vec2<float> size, float scale) {
  resize(size, scale);
}






void win::application::on_update_private() {
  update();
}
