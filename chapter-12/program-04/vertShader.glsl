#version 430
out vec2 tc;
uniform mat4 mvp_matrix;
layout(binding = 0) uniform sampler2D tex_color;
layout(binding = 1) uniform sampler2D tex_height;
void main(void)
{
	const vec2 patchTexCoords[] = vec2[](
		vec2(0, 0),
		vec2(1, 0),
		vec2(0, 1),
		vec2(1, 1));
	int x = gl_InstanceID % 64;
	int y = gl_InstanceID / 64;
	tc = vec2(
		(x + patchTexCoords[gl_VertexID].x) / 64.0,
		(63 - y + patchTexCoords[gl_VertexID].y) / 64.0);
	gl_Position = vec4(tc.x - 0.5, 0.0, (1.0 - tc.y) - 0.5, 1.0);
}
