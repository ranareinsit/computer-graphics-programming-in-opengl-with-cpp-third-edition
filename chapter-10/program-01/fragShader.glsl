#version 430
in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 originalVertex;
out vec4 fragColor;
struct PositionalLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};
struct Material
{
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
void main(void)
{
	vec3 L = normalize(varyingLightDir);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(-varyingVertPos);
	float a = 0.25;
	float b = 100.0;
	float x = originalVertex.x;
	float y = originalVertex.y;
	float z = originalVertex.z;
	N.x = varyingNormal.x + a * sin(b * x);
	N.y = varyingNormal.y + a * sin(b * y);
	N.z = varyingNormal.z + a * sin(b * z);
	N = normalize(N);
	vec3 R = normalize(reflect(-L, N));
	float cosTheta = dot(L, N);
	float cosPhi = dot(V, R);
	fragColor =
		globalAmbient *
			material.ambient +
		light.ambient *
			material.ambient +
		light.diffuse *
			material.diffuse *
			max(cosTheta, 0.0) +
		light.specular *
			material.specular *
			pow(max(cosPhi, 0.0), material.shininess);
}
