#version 450 core

layout (location=0) in vec4 position;

uniform mat4 transformA;
uniform mat4 transformB;

out vec2 uvCoordA;
out vec2 uvCoordB;

void main() {
  uvCoordA = vec2(0.5, -0.5) * (transformA * position).xy + vec2(0.5, 0.5);
  uvCoordB = vec2(0.5, -0.5) * (transformB * position).xy + vec2(0.5, 0.5);

  gl_Position = position;
}
