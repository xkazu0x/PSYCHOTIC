#version 460 core

layout (std140, binding = 0) uniform per_frame_data {
	uniform mat4 mvp;
	uniform int is_wire_frame;
};

layout (location = 0) out vec3 out_color;

const vec3 pos[8] = vec3[8](
	vec3(-1.0, -1.0, 1.0), vec3( 1.0, -1.0, 1.0),
	vec3( 1.0,  1.0, 1.0), vec3(-1.0,  1.0, 1.0),

	vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0),
	vec3( 1.0,  1.0, -1.0), vec3(-1.0,  1.0, -1.0)
);

const vec3 col[8] = vec3[8](
	vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0),

	vec3(1.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),
	vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)
);

const int indices[36] = int[36](
	// front
	0, 1, 2, 2, 3, 0,
	// right
	1, 5, 6, 6, 2, 1,
	// back
	7, 6, 5, 5, 4, 7,
	// left
	4, 0, 3, 3, 7, 4,
	// bottom
	4, 5, 1, 1, 0, 4,
	// top
	3, 2, 6, 6, 7, 3
);

void main() {
	int i = indices[gl_VertexID];
	gl_Position = mvp * vec4(pos[i], 1.0);
	out_color = is_wire_frame > 0 ? vec3(0.0) : col[i];
}