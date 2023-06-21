#ifndef GL_PROGRAM_HPP_INCLUDED
#define GL_PROGRAM_HPP_INCLUDED

#include "gl/object-name.hpp"

#include <string>
#include <string_view>



namespace gl {

class program {
  public:
    program() = default;
    program(std::string_view, std::string_view);



    [[nodiscard]] operator bool() const { return get() != 0; }

    [[nodiscard]] GLuint get() const { return program_.get(); }



    void use() const { glUseProgram(program_.get()); }

    [[nodiscard]] GLint uniform  (const std::string&) const;
    [[nodiscard]] GLint attribute(const std::string&) const;



  private:
    struct deleter {
      void operator()(GLuint p) { glDeleteProgram(p); }
    };

    object_name<deleter> program_;
};

}

#endif // GL_PROGRAM_HPP_INCLUDED
