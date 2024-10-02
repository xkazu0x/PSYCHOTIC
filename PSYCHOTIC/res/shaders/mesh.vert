#version 460 core

layout (std140, binding = 0) uniform per_frame_data {
	uniform mat4 mvp;
	uniform int is_wire_frame;
};

layout (location = 0) in vec3 in_position;

layout (location = 0) out vec3 out_color;

void main() {
	gl_Position = mvp * vec4(in_position, 1.0);
	out_color = is_wire_frame > 0 ? vec3(0.0) : in_position.xyz; 
}