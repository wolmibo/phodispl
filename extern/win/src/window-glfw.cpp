#include "win/window-glfw.hpp"

#include "win/application.hpp"
#include "win/context-glfw.hpp"

#include <stdexcept>
#include <thread>

#include <GLFW/glfw3.h>



namespace {
  [[noreturn]] void on_error(int /*error*/, const char* msg) {
    throw std::runtime_error{std::string{"glfw error: "} + msg};
  }
}



win::window_glfw::window_glfw(const std::string& app_id) {
  glfwSetErrorCallback(on_error);

  if (glfwInit() == GLFW_FALSE) {
    throw std::runtime_error("unable to initialize glfw");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED,               GLFW_TRUE);

  window_ = glfwCreateWindow(size_.x(), size_.y(), app_id.c_str(), nullptr, nullptr);
  if (window_ == nullptr) {
    throw std::runtime_error{"unable to create glfw window"};
  }

  glfwSetWindowUserPointer(window_, this);



  glfwSetFramebufferSizeCallback   (window_, &window_glfw::framebuffer_size_cb);
  glfwSetWindowContentScaleCallback(window_, &window_glfw::content_scale_cb);

  glfwSetKeyCallback (window_, &window_glfw::key_cb);
  glfwSetCharCallback(window_, &window_glfw::char_cb);

  glfwSetScrollCallback     (window_, &window_glfw::mouse_scroll_cb);
  glfwSetMouseButtonCallback(window_, &window_glfw::mouse_btn_cb);
  glfwSetCursorPosCallback  (window_, &window_glfw::mouse_pos_cb);
  glfwSetCursorEnterCallback(window_, &window_glfw::mouse_enter_cb);



  glfwMakeContextCurrent(window_);

  glfwSwapInterval(1);
}



win::window_glfw::~window_glfw() {
  glfwTerminate();
}



win::context win::window_glfw::share_context() const {
  return context{std::make_unique<context_glfw>(window_)};
}



void win::window_glfw::title(const std::string& title) {
  glfwSetWindowTitle(window_, title.c_str());
}



void win::window_glfw::close() {
  glfwSetWindowShouldClose(window_, GLFW_TRUE);
}



void win::window_glfw::run() {
  int width {0};
  int height{0};
  glfwGetFramebufferSize(window_, &width, &height);
  size_ = vec2<uint32_t>(width, height);

  float xscale{1.f};
  float yscale{1.f};
  glfwGetWindowContentScale(window_, &xscale, &yscale);
  scale_ = std::max(xscale, yscale);

  rescale(size_, scale_);

  while (glfwWindowShouldClose(window_) == GLFW_FALSE) {
    if (update()) {
      parent()->render();
      glfwSwapBuffers(window_);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }
    glfwPollEvents();
  }

  glfwHideWindow(window_);
}





bool win::window_glfw::mod_active(modifier mod) const {
  switch (mod) {
    case modifier::control:
      return glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window_, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
  }

  return false;
}





void win::window_glfw::framebuffer_size_cb(GLFWwindow* win, int width, int height) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(win));

  self->size_ = vec2<uint32_t>(width, height);

  self->rescale(self->size_, self->scale_);
}





void win::window_glfw::content_scale_cb(GLFWwindow* win, float xscale, float yscale) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(win));

  self->scale_ = std::max(xscale, yscale);

  self->rescale(self->size_, self->scale_);
}





namespace {
  [[nodiscard]] std::optional<win::key> convert_key_code(int code) {
    switch (code) {
      case GLFW_KEY_ESCAPE: return win::key::escape;
      case GLFW_KEY_HOME:   return win::key::home;
      case GLFW_KEY_LEFT:   return win::key::left;
      case GLFW_KEY_UP:     return win::key::up;
      case GLFW_KEY_RIGHT:  return win::key::right;
      case GLFW_KEY_DOWN:   return win::key::down;
      case GLFW_KEY_END:    return win::key::end;

      case GLFW_KEY_KP_ADD:      return win::key::kp_plus;
      case GLFW_KEY_KP_SUBTRACT: return win::key::kp_minus;

      case GLFW_KEY_KP_0:   return win::key::kp_0;
      case GLFW_KEY_KP_1:   return win::key::kp_1;
      case GLFW_KEY_KP_2:   return win::key::kp_2;
      case GLFW_KEY_KP_3:   return win::key::kp_3;
      case GLFW_KEY_KP_4:   return win::key::kp_4;
      case GLFW_KEY_KP_5:   return win::key::kp_5;
      case GLFW_KEY_KP_6:   return win::key::kp_6;
      case GLFW_KEY_KP_7:   return win::key::kp_7;
      case GLFW_KEY_KP_8:   return win::key::kp_8;
      case GLFW_KEY_KP_9:   return win::key::kp_9;

      case GLFW_KEY_F1:     return win::key::f1;
      case GLFW_KEY_F2:     return win::key::f2;
      case GLFW_KEY_F3:     return win::key::f3;
      case GLFW_KEY_F4:     return win::key::f4;
      case GLFW_KEY_F5:     return win::key::f5;
      case GLFW_KEY_F6:     return win::key::f6;
      case GLFW_KEY_F7:     return win::key::f7;
      case GLFW_KEY_F8:     return win::key::f8;
      case GLFW_KEY_F9:     return win::key::f9;
      case GLFW_KEY_F10:    return win::key::f10;
      case GLFW_KEY_F11:    return win::key::f11;
      case GLFW_KEY_F12:    return win::key::f12;

      default: return {};
    }
  }
}



void win::window_glfw::key_cb(
    GLFWwindow* window,
    int         key,
    int         /*scancode*/,
    int         action,
    int         /*mods*/
) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(window));

  auto prop_key = convert_key_code(key);

  self->last_key_ = key;
  if (action == GLFW_PRESS) {
    if (prop_key) {
      self->parent()->on_key_press(*prop_key);
    } else {
      self->last_key_ = key;
    }
  } else if (action == GLFW_RELEASE) {
    if (prop_key) {
      self->parent()->on_key_release(*prop_key);
    } else if (auto ix = self->key_map_.find_index(key)) {
      self->parent()->on_key_release(static_cast<win::key>(self->key_map_.value(*ix)));
    }
  }
}



void win::window_glfw::char_cb(GLFWwindow* window, unsigned int character) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(window));

  if (self->last_key_ != 0) {
    if (!self->key_map_.find_index(self->last_key_)) {
      self->key_map_.emplace(self->last_key_, character);
    }
    self->parent()->on_key_press(static_cast<win::key>(character));
  }

  self->last_key_ = 0;
}




namespace {
  [[nodiscard]] win::mouse_button convert_button(int button) {
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:   return win::mouse_button::left;
      case GLFW_MOUSE_BUTTON_RIGHT:  return win::mouse_button::right;
      case GLFW_MOUSE_BUTTON_MIDDLE: return win::mouse_button::middle;

      default:
        return static_cast<win::mouse_button>(button
            + std::to_underlying(win::mouse_button::left));
    }
  }
}



void win::window_glfw::mouse_btn_cb(
    GLFWwindow* window,
    int         button,
    int         action,
    int         /*mods*/
) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(window));

  double x{0.};
  double y{0.};
  glfwGetCursorPos(window, &x, &y);

  if (action == GLFW_PRESS) {
    self->parent()->pointer_press(vec2<float>(x, y), convert_button(button));
  } else {
    self->parent()->pointer_release(vec2<float>(x, y), convert_button(button));
  }
}



void win::window_glfw::mouse_enter_cb(GLFWwindow* window, int enter) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(window));

  if (enter != 0) {
    double x{0.};
    double y{0.};
    glfwGetCursorPos(window, &x, &y);
    self->parent()->pointer_move(vec2<float>(x, y));
  } else {
    self->parent()->pointer_leave();
  }
}



void win::window_glfw::mouse_scroll_cb(GLFWwindow* window, double dx, double dy) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(window));

  double x{0.};
  double y{0.};
  glfwGetCursorPos(window, &x, &y);

  self->parent()->scroll(vec2<float>(x, y), vec2<float>(dx * -15.f, dy * 15.f));
}



void win::window_glfw::mouse_pos_cb(GLFWwindow* window, double x, double y) {
  auto* self = static_cast<window_glfw*>(glfwGetWindowUserPointer(window));

  self->parent()->pointer_move(vec2<float>(x, y));
}





void win::window_glfw::min_size(vec2<uint32_t> size) {
  glfwSetWindowSizeLimits(window_, size.x(), size.y(), GLFW_DONT_CARE, GLFW_DONT_CARE);
}
