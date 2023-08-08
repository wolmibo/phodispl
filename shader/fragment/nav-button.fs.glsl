#version 450 core

out vec4 fragColor;

in vec2 objCoord;

uniform vec4 colorBack;
uniform vec4 colorFront;

uniform float scaleX;


void main() {
  float r = length(objCoord);
  r = (1.f - smoothstep(0.95f, 1.0f, r));

  float x = 1.6f * (scaleX * objCoord.x + 0.2f);
  float y = 2.4f * objCoord.y;
  float t = clamp(step(0.f, x) - smoothstep(0.9f, 1.0, x + abs(y)), 0.f, 1.f);
  fragColor = r * mix(colorBack, colorFront, t);
}
