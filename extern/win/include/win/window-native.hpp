#ifndef WIN_WINDOW_NATIVE_HPP_INCLUDED
#define WIN_WINDOW_NATIVE_HPP_INCLUDED

#include "win/context.hpp"
#include "win/modifier.hpp"
#include "win/vec2.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>



namespace win {

class window_listener;



enum class backend {
  none,
  glfw,
  wayland,
};

[[nodiscard]] std::string_view to_string(backend);
[[nodiscard]] backend select_backend();



class window_native {
  public:
    window_native(const window_native&) = delete;
    window_native(window_native&&)      = delete;
    window_native& operator=(const window_native&) = delete;
    window_native& operator=(window_native&&)      = delete;

    virtual ~window_native() = default;

    window_native() = default;

    [[nodiscard]] static std::unique_ptr<window_native> create(backend,
        const std::string&);



    [[nodiscard]] window_listener* listener() { return listener_; }
    void listener(window_listener*);



    [[nodiscard]] virtual win::backend backend() const { return win::backend::none; }



    virtual void run()   {}
    virtual void close() {}

    virtual void title(const std::string& /*title*/) {}

    [[nodiscard]] virtual bool mod_active(modifier /*mod*/) const { return false; }



    void rescale(vec2<uint32_t>, float);
    void damage(bool damage = true) { needs_update_ = needs_update_ || damage; }



    [[nodiscard]] virtual context share_context() const { return {}; };



  protected:
    [[nodiscard]] bool take_damage() { return std::exchange(needs_update_, false); }
    [[nodiscard]] bool update();

    virtual void on_new_listener() {}





  private:
    window_listener*  listener_    {nullptr};

    bool              needs_update_{true};
};

}

#endif // WIN_WINDOW_NATIVE_HPP_INCLUDED
