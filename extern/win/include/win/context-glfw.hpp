#ifndef WIN_CONTEXT_GLFW_HPP_INCLUDED
#define WIN_CONTEXT_GLFW_HPP_INCLUDED

#include "win/context-native.hpp"



class GLFWwindow;

namespace win {

class context_glfw : public context_native {
  public:
    context_glfw(const context_glfw&) = delete;
    context_glfw(context_glfw&&)      = delete;
    context_glfw& operator=(const context_glfw&) = delete;
    context_glfw& operator=(context_glfw&&)      = delete;

    ~context_glfw() override;

    explicit context_glfw(GLFWwindow* = nullptr);



  private:
    GLFWwindow* window_{nullptr};

    void bind()    const override;
    void release() const override;
};

}

#endif // WIN_CONTEXT_GLFW_HPP_INCLUDED
