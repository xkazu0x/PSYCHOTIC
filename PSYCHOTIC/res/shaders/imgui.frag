#version 460 core

in vec2 out_uv;
in vec4 out_color;

layout (binding = 0) uniform sampler2D texture_sampler;

out vec4 frag_color;

void main() {
	frag_color = out_color * texture(texture_sampler, out_uv.st);
}
