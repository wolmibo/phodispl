#include <gl/base.hpp>

#include "win/types.hpp"



void win::set_uniform_mat4(int uniform, const mat4& matrix) {
  glUniformMatrix4fv(uniform, 1, GL_FALSE, matrix.data());
}
