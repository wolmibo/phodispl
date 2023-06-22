#version 450 core

layout (location=0) in vec4 position;

out vec2 uvCoord;

void main() {
  uvCoord = vec2(0.5, -0.5) * position.xy + vec2(0.5, 0.5);

  gl_Position = position;
}
