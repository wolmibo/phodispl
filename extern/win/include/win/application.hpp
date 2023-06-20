#ifndef WIN_APPLICATION_HPP_INCLUDED
#define WIN_APPLICATION_HPP_INCLUDED

#include "win/types.hpp"
#include "win/viewport.hpp"
#include "win/window-listener.hpp"
#include "win/window-native.hpp"

#include <chrono>
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



    [[nodiscard]] uint32_t       width()   const { return native_->width();  }
    [[nodiscard]] uint32_t       height()  const { return native_->height(); }
    [[nodiscard]] vec2<uint32_t> size()    const { return native_->size();   }
    [[nodiscard]] float          scale()   const { return native_->scale();  }

    [[nodiscard]] win::backend   backend() const { return native_->backend(); }



    [[nodiscard]] color background_color() const { return bg_color_; }
    void background_color(color c) { bg_color_ = c; damage(); }



    void run()   { native_->run();   }
    void close() { native_->close(); }

    void title(const std::string& title) { native_->title(title); }

    [[nodiscard]] context share_context() const { return native_->share_context(); }

    [[nodiscard]] bool mod_active(modifier mod) const { return native_->mod_active(mod); }



    [[nodiscard]] uint64_t elapsed() const;





  protected:
    void damage(bool damage = true) { native_->damage(damage); }





  private:
    std::chrono::steady_clock::time_point start_;
    std::unique_ptr<window_native>        native_;

    color                                 bg_color_{0.f, 0.f, 0.f, 1.f};

    void on_render_private() final;
    void on_resize_private(vec2<float> /*size*/, float /*scale*/) final;
};

}


#endif // WIN_APPLICATION_HPP_INCLUDED
