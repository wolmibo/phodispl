#version 450 core

out vec4 fragColor;

in vec2 uvCoord;
uniform sampler2D textureSampler;

uniform vec4 factor;
uniform vec4 exponent;

void main() {
  fragColor = factor * pow(texture(textureSampler, uvCoord), exponent);
}
