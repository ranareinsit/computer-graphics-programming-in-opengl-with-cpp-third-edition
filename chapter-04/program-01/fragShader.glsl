#version 410
in vec4 varyingColor;
out vec4 color;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
void main(void) {
	color = varyingColor;
}
/*
#version 410
out vec4 color;
void main(void) {
    ivec2 coord = ivec2(gl_FragCoord.x, gl_FragCoord.y);
    float xorValue = float((coord.x ^ coord.y) & 255) / 255.0;
    color = vec4(vec3(xorValue), 1.0);
}
*/