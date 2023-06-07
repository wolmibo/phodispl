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
