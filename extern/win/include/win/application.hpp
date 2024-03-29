#ifndef WIN_APPLICATION_HPP_INCLUDED
#define WIN_APPLICATION_HPP_INCLUDED

#include "win/viewport.hpp"
#include "win/window-listener.hpp"
#include "win/window-native.hpp"

#include <memory>



namespace win {

class application : public window_listener, public viewport {
  public:
    application(const application&) = delete;
    application(application&&) noexcept;

    application& operator=(const application&) = delete;
    application& operator=(application&&) noexcept;

    ~application() override = default;



    explicit application(const std::string& /*app_id*/);



    [[nodiscard]] win::backend backend() const { return native_->backend(); }

    void run() { native_->run();   }



    [[nodiscard]] const window_native& window() const { return *native_; }
    [[nodiscard]]       window_native& window()       { return *native_; }



  private:
    std::unique_ptr<window_native>        native_;

    void on_resize_private(vec2<float> /*size*/, float /*scale*/) final;
    void on_update_private() final;
};

}


#endif // WIN_APPLICATION_HPP_INCLUDED
