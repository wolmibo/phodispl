#version 450 core

out vec4 fragColor;

in vec2 uvCoord;
uniform sampler2D textureSampler;

uniform float alpha;

void main() {
	fragColor = alpha * texture(textureSampler, uvCoord);
}