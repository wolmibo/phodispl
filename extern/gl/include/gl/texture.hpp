#ifndef GL_TEXTURE_HPP_INCLUDED
#define GL_TEXTURE_HPP_INCLUDED

#include "gl/object-name.hpp"



namespace gl {

class texture {
  public:
    explicit texture(GLuint texture = 0) : texture_{texture} {}



    [[nodiscard]] GLuint   get()  const noexcept { return texture_.get(); }
    [[nodiscard]] operator bool() const noexcept { return get() != 0;     }

    void bind() const { glBindTexture(GL_TEXTURE_2D, texture_.get()); }



    [[nodiscard]] friend bool operator==(const texture& l, const texture& r) noexcept {
      return l.get() == r.get() && l.get() != 0;
    }



  private:
    struct deleter {
      void operator()(GLuint t) { glDeleteTextures(1, &t); }
    };

    object_name<deleter> texture_;
};

}


#endif // GL_TEXTURE_HPP_INCLUDED
