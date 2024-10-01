#version 460 core

layout (location = 0) out vec3 out_color;

const vec2 pos[3] = vec2[3](
	vec2(-0.5, -0.5),
	vec2( 0.5, -0.5),
	vec2( 0.0,  0.5)
);

const vec3 col[3] = vec3[3](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

void main() {
	gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
	out_color = col[gl_VertexID];
}