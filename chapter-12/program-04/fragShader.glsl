#version 430

in vec2 tes_out;
out vec4 color;
uniform mat4 mvp_matrix;

layout (binding = 0) uniform sampler2D tex_color;
layout (binding = 1) uniform sampler2D tex_height;

void main(void) {
	color = texture(tex_color, tes_out);
}

/*
#version 430

in vec2 tes_out;
out vec4 color;
uniform mat4 mvp_matrix;

layout (binding = 0) uniform sampler2D tex_color;
layout (binding = 1) uniform sampler2D tex_height;

void main(void) {
    vec4 texColor = texture(tex_color, tes_out); // Sample the color texture
    ivec2 coord = ivec2(tes_out * vec2(textureSize(tex_color, 0))); // Use texture coordinates to get a position in texture space
    float xorValue = float((coord.x ^ coord.y) & 255) / 255.0; // XOR based on texture coordinates
    vec4 xorColor = vec4(vec3(xorValue), 1.0); // XOR color

    // Mix the XOR color with the texture color based on some factor
    color = mix(texColor, xorColor, 0.5); // 0.5 is the mix factor, adjust as needed
}
*/