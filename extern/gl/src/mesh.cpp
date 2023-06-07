#include "gl/mesh.hpp"

#include <stdexcept>



namespace {
  [[nodiscard]] GLuint generate_vertex_array() {
    GLuint vao{0};

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    return vao;
  }



  [[nodiscard]] GLuint generate_vertex_buffer(std::span<const GLfloat> vertices) {
    GLuint vbo{0};

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    return vbo;
  }



  [[nodiscard]] GLuint generate_index_buffer(std::span<const GLushort> indices) {
    GLuint ibo{0};

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size_bytes(), indices.data(), GL_STATIC_DRAW);

    return ibo;
  }
}



gl::mesh::mesh(std::span<const GLfloat> vertices, std::span<const GLushort> indices) :
  vao_{generate_vertex_array()},
  vbo_{generate_vertex_buffer(vertices)},
  ibo_{generate_index_buffer(indices)},

  element_count_{indices.size()}
{}





gl::mesh::mesh(GLuint vao, GLuint vbo, GLuint ibo) :
  vao_{vao},
  vbo_{vbo},
  ibo_{ibo},

  element_count_{get_element_count(ibo)}
{}





void gl::mesh::draw() const {
  glBindVertexArray(vao_.get());
  glDrawElements(GL_TRIANGLES, element_count_, GL_UNSIGNED_SHORT, nullptr);
}





size_t gl::mesh::get_element_count(GLuint ibo) {
  GLint64 byte_size{0};

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glGetBufferParameteri64v(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &byte_size);

  if (byte_size < 0) {
    throw std::runtime_error{"unable to obtain element count"};
  }

  if (byte_size % sizeof(GLushort) != 0) {
    throw std::runtime_error{"element count is not an integer"};
  }

  return byte_size / sizeof(GLushort);
}
