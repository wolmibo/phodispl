#ifndef WIN_WINDOW_GLFW_HPP_INCLUDED
#define WIN_WINDOW_GLFW_HPP_INCLUDED

#include "win/window-native.hpp"

#include <string>

#include <chaos-map.hpp>



class GLFWwindow;

namespace win {

class window_glfw : public window_native {
  public:
    window_glfw(const window_glfw&) = delete;
    window_glfw(window_glfw&&)      = delete;
    window_glfw& operator=(const window_glfw&) = delete;
    window_glfw& operator=(window_glfw&&)      = delete;

    ~window_glfw() override;

    explicit window_glfw(const std::string&);



  private:
    GLFWwindow*              window_    {nullptr};

    int                      old_pos_x_ {0};
    int                      old_pos_y_ {0};
    int                      old_width_ {0};
    int                      old_height_{0};

    int                      last_key_  {0};
    chaos_map<int, uint32_t> key_map_;



    [[nodiscard]] win::backend backend() const override { return win::backend::glfw; }



    void title(const std::string& /*title*/) override;
    void close() override;
    void run()   override;

    [[nodiscard]] bool mod_active(modifier /*mod*/) const override;

    [[nodiscard]] context share_context() const override;



    void min_size(vec2<uint32_t> /*size*/) override;



    static void framebuffer_size_cb(GLFWwindow*, int, int);

    static void char_cb            (GLFWwindow*, unsigned int);
    static void key_cb             (GLFWwindow*, int, int, int, int);

    static void mouse_btn_cb       (GLFWwindow*, int, int, int);
    static void mouse_enter_cb     (GLFWwindow*, int);
    static void mouse_pos_cb       (GLFWwindow*, double, double);
    static void mouse_scroll_cb    (GLFWwindow*, double, double);
};

}

#endif // WIN_WINDOW_GLFW_HPP_INCLUDED
