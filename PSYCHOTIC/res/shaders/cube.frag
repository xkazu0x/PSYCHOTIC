#version 460 core

layout (location = 0) in vec2 out_uv;
layout (location = 1) in vec3 out_color;

layout (binding = 0) uniform sampler2D texture_sampler;

layout (location = 0) out vec4 frag_color;

void main() {
	//frag_color = texture(texture_sampler, out_uv);
	frag_color = vec4(out_color, 1.0) * texture(texture_sampler, out_uv);
}