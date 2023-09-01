#version 450 core

out vec4 fragColor;

in vec2 objCoord;
in vec2 icoCoord;

uniform vec4 colorBack;
uniform vec4 colorFront;

uniform float aaSize;


void main() {
  float r = length(objCoord);
  float outline = smoothstep(0.95f - 0.05f * aaSize, 0.95f, r);
  r = (1.f - smoothstep(1.f - 0.05f * aaSize, 1.f, r));

  float t = step(0.f, icoCoord.x)
            - smoothstep(1.f - 0.1f * aaSize, 1.0, icoCoord.x + abs(icoCoord.y));

  fragColor = r * mix(colorBack, colorFront, max(t, outline));
}
