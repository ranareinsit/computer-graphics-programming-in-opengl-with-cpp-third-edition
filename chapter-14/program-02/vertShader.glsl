#version 430
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNormal;
out vec3 vNormal, vLightDir, vVertPos, vHalfVec;
struct PositionalLight
{
	vec4 ambient, diffuse, specular;
	vec3 position;
};
struct Material
{
	vec4 ambient, diffuse, specular;
	float shininess;
};
uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform float alpha;
uniform float flipNormal;
void main(void)
{
	vVertPos = (mv_matrix * vec4(vertPos, 1.0)).xyz;
	vLightDir = light.position - vVertPos;
	vNormal = (norm_matrix * vec4(vertNormal, 1.0)).xyz;
	if (flipNormal < 0)
		vNormal = -vNormal;
	vHalfVec = (vLightDir - vVertPos).xyz;
	gl_Position = proj_matrix * mv_matrix * vec4(vertPos, 1.0);
}
