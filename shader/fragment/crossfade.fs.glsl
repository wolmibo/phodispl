#version 450 core

out vec4 fragColor;

in vec2 uvCoordA;
in vec2 uvCoordB;

uniform sampler2D textureSamplerA;
uniform sampler2D textureSamplerB;

uniform vec4 factorA;
uniform vec4 factorB;

void main() {
  fragColor = factorA * texture(textureSamplerA, uvCoordA)
              + factorB * texture(textureSamplerB, uvCoordB);
}
