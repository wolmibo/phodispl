#version 450 core

layout (location=0) in vec4 position;
layout (location=0) uniform mat4 transform;

out vec2 objCoord;

void main() {
  objCoord = position.xy;
  gl_Position = transform * position;
}