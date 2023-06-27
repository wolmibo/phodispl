#version 450 core

layout (location=0) in vec4 position;
layout (location=0) uniform mat4 transform;

uniform mat4 transformContinuous;

out vec2 objCoord;
out vec2 conCoord;

void main() {
  objCoord = position.xy;
  conCoord = (transformContinuous * position).xy;
  gl_Position = transform * position;
}
