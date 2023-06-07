#include "gl/base.hpp"

#include <logcerr/log.hpp>



namespace {
  template<typename T = void> requires (logcerr::debugging_enabled())
  [[nodiscard]] std::string_view gl_message_source(GLenum source) {
    switch (source) {
      case GL_DEBUG_SOURCE_API:             return "API";
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "window system";
      case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader compiler";
      case GL_DEBUG_SOURCE_THIRD_PARTY:     return "3rd party";
      case GL_DEBUG_SOURCE_APPLICATION:     return "application";

      case GL_DEBUG_SOURCE_OTHER:
      default:
        return "unknown source";
    }
  }



  template<typename T = void> requires (logcerr::debugging_enabled())
  [[nodiscard]] std::string_view gl_message_type(GLenum type) {
    switch (type) {
      case GL_DEBUG_TYPE_ERROR:               return "an error";
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated behavior";
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined behavior";
      case GL_DEBUG_TYPE_PORTABILITY:         return "a portability issue";
      case GL_DEBUG_TYPE_PERFORMANCE:         return "a performance issue";
      case GL_DEBUG_TYPE_MARKER:              return "an annotation";
      case GL_DEBUG_TYPE_PUSH_GROUP:          return "group pushing";
      case GL_DEBUG_TYPE_POP_GROUP:           return "group popping";

      case GL_DEBUG_TYPE_OTHER:
      default:                                return "something";
    }
  }



  template<typename T = void> requires (logcerr::debugging_enabled())
  [[nodiscard]] logcerr::severity gl_severity(GLenum severity) {
    switch (severity) {
      case GL_DEBUG_SEVERITY_HIGH:         return logcerr::severity::error;
      case GL_DEBUG_SEVERITY_MEDIUM:       return logcerr::severity::warning;

      case GL_DEBUG_SEVERITY_LOW:
      case GL_DEBUG_SEVERITY_NOTIFICATION:
      default:                             return logcerr::severity::debug;
    }
  }



  template<typename T = void> requires (logcerr::debugging_enabled())
  void GLAPIENTRY message_callback(
      GLenum        source,
      GLenum        type,
      GLuint        id,
      GLenum        severity,
      GLsizei       /*length*/,
      const GLchar* message,
      const void*   /*userptr*/
  ) {
    logcerr::print(gl_severity(severity), "GL {} reports {} ({}):\n{}",
        gl_message_source(source), gl_message_type(type), id, message);
  }
}



void gl::enable_debugging() {
  if constexpr (logcerr::debugging_enabled()) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(&message_callback, nullptr);
  }
}
