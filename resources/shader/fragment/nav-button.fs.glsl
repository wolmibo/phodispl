#version 450 core

out vec4 fragColor;

in vec2 objCoord;
in vec2 icoCoord;

uniform vec4 colorBack;
uniform vec4 colorFront;


void main() {
  float r = length(objCoord);
  float outline = smoothstep(0.90f, 0.95f, r);
  r = (1.f - smoothstep(0.95f, 1.f, r));

  float t = step(0.f, icoCoord.x) - smoothstep(0.9f, 1.0, icoCoord.x + abs(icoCoord.y));

  fragColor = r * mix(colorBack, colorFront, max(t, outline));
}
