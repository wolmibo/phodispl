#version 450 core

out vec4 fragColor;

in vec2 objCoord;
in vec2 conCoord;

uniform float value;
uniform float alpha;

const float over_2pi = 0.15915494309189535;

float value_ring(vec2 coord, float v) {
  float phi = 0.5 - atan(coord.x, -coord.y) * over_2pi;

  float r = length(coord);
  r = (smoothstep(0.72, 0.75, r) - smoothstep(0.97, 1.0, r));


  return (smoothstep(phi-0.01, phi, v) + 0.2) * r;
}

void main() {
  fragColor = (value_ring(objCoord, value) + value_ring(conCoord, 0.2)) * alpha
    * vec4(0.2, 0.2, 0.2, 0.2);
}