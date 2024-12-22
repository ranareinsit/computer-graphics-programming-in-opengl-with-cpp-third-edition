#version 430

in vec3 vertEyeSpacePos;
in vec2 tc;
out vec4 fragColor;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
layout (binding=0) uniform sampler2D t;	
layout (binding=1) uniform sampler2D h;	

void main(void) {
	vec4 fogColor = vec4(0.7, 0.8, 0.9, 1.0);
	float fogStart = 0.2;
	float fogEnd = 0.8;
	float dist = length(vertEyeSpacePos.xyz);
	float fogFactor = clamp(((fogEnd-dist)/(fogEnd-fogStart)), 0.0, 1.0);
	fragColor = mix(fogColor,(texture(t,tc)),fogFactor);
}
