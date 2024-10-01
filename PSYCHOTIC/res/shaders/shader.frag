#version 460 core

layout (location = 0) in vec3 out_color;
layout (location = 1) in vec2 out_uv;

layout (location = 0) out vec4 frag_color;

uniform sampler2D texture0;

void main() {
	//frag_color = vec4(out_color, 1.0);
	frag_color = texture(texture0, out_uv);
}