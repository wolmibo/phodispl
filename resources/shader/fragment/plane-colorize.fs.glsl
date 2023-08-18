#version 450 core

out vec4 fragColor;

in vec2 uvCoord;
uniform sampler2D textureSampler;

uniform vec4 color;

void main() {
	fragColor = texture(textureSampler, uvCoord).r * color;
}