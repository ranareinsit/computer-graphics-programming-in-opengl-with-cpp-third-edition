#version 430
in vec3 varyingNormalG;
in vec3 varyingLightDirG;
in vec3 varyingHalfVectorG;
out vec4 fragColor;
struct PositionalLight {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};
struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
void main(void) {
	vec3 L = normalize(varyingLightDirG);
	vec3 N = normalize(varyingNormalG);
	float cosTheta = dot(L,N);
	vec3 H = normalize(varyingHalfVectorG);
	fragColor = 
		globalAmbient * 
		material.ambient + 
		light.ambient * 
		material.ambient + 
		light.diffuse * 
		material.diffuse * 
		max(cosTheta,0.0) + 
		light.specular * 
		material.specular * 
		pow(max(dot(H,N),0.0), material.shininess*3.0);
}
