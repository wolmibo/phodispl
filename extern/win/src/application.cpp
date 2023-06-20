#include <gl/base.hpp>

#include "win/application.hpp"
#include "win/window-glfw.hpp"





win::application::application(application&& rhs) noexcept :
  start_  {rhs.start_},
  native_ {std::move(rhs.native_)}
{
  native_->listener(this);
}



win::application& win::application::operator=(application&& rhs) noexcept {
  start_ = rhs.start_;

  native_->listener(nullptr);
  native_ = std::move(rhs.native_);
  native_->listener(this);

  return *this;
}



uint64_t win::application::elapsed() const {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - start_).count();
}





win::application::application(const std::string& app_id) :
  start_ {std::chrono::steady_clock::now()},
  native_{window_native::create(win::select_backend(), app_id)}
{
  native_->listener(this);
}





void win::application::on_render_private() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(
      bg_color_[0] * bg_color_[3],
      bg_color_[1] * bg_color_[3],
      bg_color_[2] * bg_color_[3],
      bg_color_[3]
  );

  glClear(GL_COLOR_BUFFER_BIT);

  render();
}



void win::application::on_resize_private(vec2<float> size, float scale) {
  resize(size, scale);
}
