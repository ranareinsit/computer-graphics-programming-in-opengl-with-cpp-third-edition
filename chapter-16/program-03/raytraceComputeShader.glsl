#version 430
layout(local_size_x = 1) in;
layout(binding = 0, rgba8) uniform image2D output_texture;
float camera_pos = 5.0;
struct Ray
{
	vec3 start;
	vec3 dir;
};
float sphere_radius = 2.5;
vec3 sphere_position = vec3(1.0, 0.0, -3.0);
vec3 sphere_color = vec3(1.0, 0.0, 0.0);
vec3 box_mins = vec3(-2.0, -2.0, 0.0);
vec3 box_maxs = vec3(-0.5, 1.0, 2.0);
vec3 box_color = vec3(0.0, 1.0, 0.0);
vec4 worldAmb_ambient = vec4(0.3, 0.3, 0.3, 1.0);
vec4 objMat_ambient = vec4(0.2, 0.2, 0.2, 1.0);
vec4 objMat_diffuse = vec4(0.7, 0.7, 0.7, 1.0);
vec4 objMat_specular = vec4(1.0, 1.0, 1.0, 1.0);
float objMat_shininess = 50.0;
vec3 pointLight_position = vec3(-3.0, 2.0, 4.0);
vec4 pointLight_ambient = vec4(0.2, 0.2, 0.2, 1.0);
vec4 pointLight_diffuse = vec4(0.7, 0.7, 0.7, 1.0);
vec4 pointLight_specular = vec4(1.0, 1.0, 1.0, 1.0);
struct Collision
{
	float t;
	vec3 p;
	vec3 n;
	bool inside;
	int object_index;
};

Collision intersect_box_object(Ray r)
{

	vec3 t_min = (box_mins - r.start) / r.dir;
	vec3 t_max = (box_maxs - r.start) / r.dir;
	vec3 t_minDist = min(t_min, t_max);
	vec3 t_maxDist = max(t_min, t_max);
	float t_near = max(max(t_minDist.x, t_minDist.y), t_minDist.z);
	float t_far = min(min(t_maxDist.x, t_maxDist.y), t_maxDist.z);
	Collision c;
	c.t = t_near;
	c.inside = false;

	if (t_near >= t_far || t_far <= 0.0)
	{
		c.t = -1.0;
		return c;
	}
	float intersect_distances = t_near;
	vec3 plane_intersect_distances = t_minDist;

	if (t_near < 0.0)
	{
		c.t = t_far;
		intersect_distances = t_far;
		plane_intersect_distances = t_maxDist;
		c.inside = true;
	}

	int face_index = 0;
	if (intersect_distances == plane_intersect_distances.y)
		face_index = 1;
	else if (intersect_distances == plane_intersect_distances.z)
		face_index = 2;

	c.n = vec3(0.0);
	c.n[face_index] = 1.0;

	if (r.dir[face_index] > 0.0)
		c.n *= -1.0;

	c.p = r.start + c.t * r.dir;
	return c;
}

Collision intersect_sphere_object(Ray r)
{
	float qa = dot(r.dir, r.dir);
	float qb = dot(2 * r.dir, r.start - sphere_position);
	float qc = dot(r.start - sphere_position, r.start - sphere_position) - sphere_radius * sphere_radius;

	float qd = qb * qb - 4 * qa * qc;
	Collision c;
	c.inside = false;
	if (qd < 0.0)
	{
		c.t = -1.0;
		return c;
	}
	float t1 = (-qb + sqrt(qd)) / (2.0 * qa);
	float t2 = (-qb - sqrt(qd)) / (2.0 * qa);
	float t_near = min(t1, t2);
	float t_far = max(t1, t2);
	c.t = t_near;
	if (t_far < 0.0)
	{
		c.t = -1.0;
		return c;
	}
	if (t_near < 0.0)
	{
		c.t = t_far;
		c.inside = true;
	}
	c.p = r.start + c.t * r.dir;
	c.n = normalize(c.p - sphere_position);
	if (c.inside)
	{
		c.n *= -1.0;
	}
	return c;
}

Collision get_closest_collision(Ray r)
{
	Collision closest_collision, cSph, cBox;
	closest_collision.object_index = -1;
	cSph = intersect_sphere_object(r);
	cBox = intersect_box_object(r);

	if ((cSph.t > 0) && ((cSph.t < cBox.t) || (cBox.t < 0)))
	{
		closest_collision = cSph;
		closest_collision.object_index = 1;
	}
	if ((cBox.t > 0) && ((cBox.t < cSph.t) || (cSph.t < 0)))
	{
		closest_collision = cBox;
		closest_collision.object_index = 2;
	}
	return closest_collision;
}
vec3 ads_phong_lighting(Ray r, Collision c)
{

	vec4 ambient = worldAmb_ambient + pointLight_ambient * objMat_ambient;
	vec4 diffuse = vec4(0.0);
	vec4 specular = vec4(0.0);

	Ray light_ray;
	light_ray.start = c.p + c.n * 0.01;
	light_ray.dir = normalize(pointLight_position - c.p);
	bool in_shadow = false;

	Collision c_shadow = get_closest_collision(light_ray);

	if (c_shadow.object_index != -1 && c_shadow.t < length(pointLight_position - c.p))
	{
		in_shadow = true;
	}

	if (in_shadow == false)
	{

		vec3 light_dir = normalize(pointLight_position - c.p);
		vec3 light_ref = normalize(reflect(-light_dir, c.n));
		float cos_theta = dot(light_dir, c.n);
		float cos_phi = dot(normalize(-r.dir), light_ref);
		diffuse = pointLight_diffuse * objMat_diffuse * max(cos_theta, 0.0);
		specular = pointLight_specular * objMat_specular * pow(max(cos_phi, 0.0), objMat_shininess);
	}
	vec4 phong_color = ambient + diffuse + specular;
	return phong_color.rgb;
}
vec3 raytrace(Ray r)
{
	Collision c = get_closest_collision(r);
	if (c.object_index == -1)
		return vec3(0.0);
	if (c.object_index == 1)
		return ads_phong_lighting(r, c) * sphere_color;
	if (c.object_index == 2)
		return ads_phong_lighting(r, c) * box_color;
}
void main()
{
	int width = int(gl_NumWorkGroups.x);
	int height = int(gl_NumWorkGroups.y);
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

	float x_pixel = 2.0 * pixel.x / width - 1.0;
	float y_pixel = 2.0 * pixel.y / height - 1.0;
	Ray world_ray;
	world_ray.start = vec3(0.0, 0.0, camera_pos);
	vec4 world_ray_end = vec4(x_pixel, y_pixel, camera_pos - 1.0, 1.0);
	world_ray.dir = normalize(world_ray_end.xyz - world_ray.start);

	vec3 color = raytrace(world_ray);
	imageStore(output_texture, pixel, vec4(color, 1.0));
}