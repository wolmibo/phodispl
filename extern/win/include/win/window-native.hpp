#ifndef WIN_WINDOW_NATIVE_HPP_INCLUDED
#define WIN_WINDOW_NATIVE_HPP_INCLUDED

#include "win/context.hpp"
#include "win/modifier.hpp"
#include "win/vec2.hpp"

#include <cstdint>
#include <memory>
#include <string>



namespace win {

class application;



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



    [[nodiscard]] application* parent() { return parent_; }
    void parent(application*);



    [[nodiscard]] virtual win::backend backend() const { return win::backend::none; }



    virtual void run()   {}
    virtual void close() {}

    virtual void title(const std::string& /*title*/) {}

    [[nodiscard]] virtual bool mod_active(modifier /*mod*/) const { return false; }



    void rescale(vec2<uint32_t>, float);

    virtual void min_size(vec2<uint32_t> /*size*/) {}



    [[nodiscard]] virtual context share_context() const { return {}; };



  protected:
    [[nodiscard]] bool update();

    virtual void on_new_parent() {}





  private:
    application*  parent_    {nullptr};
};

}

#endif // WIN_WINDOW_NATIVE_HPP_INCLUDED
