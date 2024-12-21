#version 430

layout (quads, equal_spacing,ccw) in;

uniform mat4 mvp_matrix;
layout (binding = 0) uniform sampler2D tex_color;

in vec2 tcs_out[];
out vec2 tes_out;

void main (void) {
	vec3 p00 = (gl_in[0].gl_Position).xyz;
	vec3 p10 = (gl_in[1].gl_Position).xyz;
	vec3 p20 = (gl_in[2].gl_Position).xyz;
	vec3 p30 = (gl_in[3].gl_Position).xyz;
	vec3 p01 = (gl_in[4].gl_Position).xyz;
	vec3 p11 = (gl_in[5].gl_Position).xyz;
	vec3 p21 = (gl_in[6].gl_Position).xyz;
	vec3 p31 = (gl_in[7].gl_Position).xyz;
	vec3 p02 = (gl_in[8].gl_Position).xyz;
	vec3 p12 = (gl_in[9].gl_Position).xyz;
	vec3 p22 = (gl_in[10].gl_Position).xyz;
	vec3 p32 = (gl_in[11].gl_Position).xyz;
	vec3 p03 = (gl_in[12].gl_Position).xyz;
	vec3 p13 = (gl_in[13].gl_Position).xyz;
	vec3 p23 = (gl_in[14].gl_Position).xyz;
	vec3 p33 = (gl_in[15].gl_Position).xyz;
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float bu0 = (1.0-u)  * (1.0-u)  * (1.0-u);	
	float bu1 = 3.0 * u * (1.0-u) * (1.0-u);	
	float bu2 = 3. * u * u * (1.0-u);			
	float bu3 = u * u * u;						
	float bv0 = (1.0-v)  * (1.0-v)  * (1.0-v);	
	float bv1 = 3.0 * v * (1.0-v) * (1.0-v);	
	float bv2 = 3. * v * v * (1.0-v);			
	float bv3 = v * v * v;						
	vec3 outputPosition =
		  bu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 ) +
		  bu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 ) +
		  bu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 ) +
		  bu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 );
	gl_Position = mvp_matrix * vec4(outputPosition,1.0f);   
//	gl_Position = mvp_matrix * vec4(u,0,v,1);               
	
	vec2 tc1 = mix(tcs_out[0], tcs_out[3], gl_TessCoord.x);
	vec2 tc2 = mix(tcs_out[12], tcs_out[15], gl_TessCoord.x);
	vec2 tc = mix(tc2, tc1, gl_TessCoord.y);
	tes_out = tc;
}