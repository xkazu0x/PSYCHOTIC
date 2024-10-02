#version 460 core

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

layout(std140, binding = 0) uniform PerFrameData {
	uniform mat4 mvp;
};

out vec2 out_uv;
out vec4 out_color;

void main() {
	out_uv = in_uv;
	out_color = in_color;
	gl_Position = mvp * vec4(in_position.xy, 0.0, 1.0);
}