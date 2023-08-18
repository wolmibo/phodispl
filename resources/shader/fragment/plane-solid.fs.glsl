#version 450 core

out vec4 fragColor;

in vec2 objCoord;

uniform vec4 color;

void main() {
  fragColor = color;
}