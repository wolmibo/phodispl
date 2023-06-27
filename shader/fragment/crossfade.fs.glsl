#version 450 core

out vec4 fragColor;

in vec2 uvCoordA;
in vec2 uvCoordB;

uniform sampler2D textureSamplerA;
uniform sampler2D textureSamplerB;

uniform vec4 factorA;
uniform vec4 factorB;

void main() {
  vec4 colorA = vec4(0.f, 0.f, 0.f, 0.f);
  if (all(greaterThan(uvCoordA, vec2(0.f))) && all(lessThan(uvCoordA, vec2(1.f)))) {
    colorA = factorA * texture(textureSamplerA, uvCoordA);
  }

  vec4 colorB = vec4(0.f, 0.f, 0.f, 0.f);
  if (all(greaterThan(uvCoordB, vec2(0.f))) && all(lessThan(uvCoordB, vec2(1.f)))) {
    colorB = factorB * texture(textureSamplerB, uvCoordB);
  }

  fragColor = colorA + colorB;
}
