#include "win/context-glfw.hpp"

#include <stdexcept>

#include <GLFW/glfw3.h>



namespace {
  [[nodiscard]] GLFWwindow* create_window(GLFWwindow* parent) {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(100, 100, "", nullptr, parent);

    if (window == nullptr) {
      throw std::runtime_error{"unable to create glfw background context"};
    }

    return window;
  }
}



win::context_glfw::context_glfw(GLFWwindow* parent) :
  window_{create_window(parent)}
{}



win::context_glfw::~context_glfw() {
  if (window_ != nullptr && glfwGetCurrentContext() == window_) {
    glfwMakeContextCurrent(nullptr);
  }
}



void win::context_glfw::bind() const {
  glfwMakeContextCurrent(window_);
}



void win::context_glfw::release() const {
  glfwMakeContextCurrent(nullptr);
}
