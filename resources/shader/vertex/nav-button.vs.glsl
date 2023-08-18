#version 450 core

layout (location=0) in vec4 position;
layout (location=0) uniform mat4 transform;

uniform float scaleX;
uniform float scaleR;

out vec2 objCoord;
out vec2 icoCoord;

void main() {
  objCoord = scaleR * position.xy;
  icoCoord = vec2(1.6f * scaleR * (position.x * scaleX + 0.2f), 2.4f * objCoord.y);
  gl_Position = transform * position;
}
