#version 450 core

out vec4 fragColor;

in vec2 uvCoord;
uniform sampler2D textureSampler;

uniform float alpha;
uniform vec4  exponent;

void main() {
  fragColor = alpha * pow(texture(textureSampler, uvCoord), exponent);
}
