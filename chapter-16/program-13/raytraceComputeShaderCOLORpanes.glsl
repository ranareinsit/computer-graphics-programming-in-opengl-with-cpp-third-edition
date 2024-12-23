#version 430
layout (local_size_x=1) in;
layout (binding=0, rgba8) uniform image2D output_texture;
layout (binding=1) uniform sampler2D sampMarble;
struct Object {
	int       type;            
	float     radius;          
	vec3      mins;            
	vec3      maxs;            
	float     xrot;            
	float     yrot;            
	float     zrot;            
	vec3      position;        
	bool      hasColor;        
	bool      hasTexture;      
	bool      isReflective;    
	bool      isRefractive;    
	vec3      color;           
	float     reflectivity;    
	float     refractivity;    
	float     IOR;             
	vec4      ambient;         
	vec4      diffuse;         
	vec4      specular;        
	float     shininess;       
};

Object[] objects = {
	
	{ 0, 0.0, vec3(-20, -20, -20), vec3( 20, 20, 20), 0, 0, 0, vec3(0),
		true, false, false, false, vec3(1,1,1), 0, 0, 0,
		vec4(0.2, 0.2, 0.2, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(1,1,1,1), 50.0
	},
	
	{ 3, 0, vec3(4, 0, 4), vec3(0), 90.0, 0.0, 0.0, vec3(-1.5, 0, 0),
	   true, false, false, true, vec3(1,0,1), 0.0, 0.5, 0.9, 
	   vec4(0.4, 0.4, 0.4, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(1,1,1,1), 100.0
	},
	
	{ 3, 0, vec3(4, 0, 4), vec3(0), 90.0, 0.0, 0.0, vec3(1.5, 1.0, 0.2),
	   true, false, false, true, vec3(1,1,0), 0.0, 0.5, 0.9, 
	   vec4(0.4, 0.4, 0.4, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(1,1,1,1), 100.0
	},
	
	{ 3, 0, vec3(4, 0, 4), vec3(0), 90.0, 0.0, 0.0, vec3(0, -1.5, 0.4),
	   true, false, false, true, vec3(0,1,1), 0.0, 0.5, 0.9, 
	   vec4(0.4, 0.4, 0.4, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(1,1,1,1), 100.0
	},
};
int numObjects = 4;
float camera_pos = 5.0;
const int max_depth = 4;
const int stack_size = 100;
vec3 pointLight_position = vec3(-1,1,3);
	   
struct Ray {
	vec3 start;	
	vec3 dir;	
};
const float PI = 3.14159265358;
const float DEG_TO_RAD = PI / 180.0;
vec4 worldAmb_ambient = vec4(0.3, 0.3, 0.3, 1.0);
vec4 pointLight_ambient = vec4(0.4, 0.4, 0.4, 1.0);
vec4 pointLight_diffuse = vec4(0.7, 0.7, 0.7, 1.0);
vec4 pointLight_specular = vec4(1.0, 1.0, 1.0, 1.0);
struct Collision {
	float t;	
	vec3 p;		
	vec3 n;		
	bool inside;	
	int object_index;	
	vec2 tc;	
	int face_index; 
};

struct Stack_Element {
	int type;	
	int depth;	
	int phase;	
	vec3 phong_color;		
	vec3 reflected_color;	
	vec3 refracted_color;	
	vec3 final_color;		
	Ray ray;				
	Collision collision;	
};
const int RAY_TYPE_REFLECTION = 1;
const int RAY_TYPE_REFRACTION = 2;
Ray null_ray = {vec3(0.0), vec3(0.0)};
Collision null_collision = { -1.0, vec3(0.0), vec3(0.0), false, -1, vec2(0.0, 0.0), -1 };
Stack_Element null_stack_element = {0,-1,-1,vec3(0),vec3(0),vec3(0),vec3(0),null_ray,null_collision};
Stack_Element stack[stack_size];
int stack_pointer = -1;			
Stack_Element popped_stack_element;	

mat4 buildTranslate(float x, float y, float z) {
	return mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, x, y, z, 1.0);
}
mat4 buildRotateX(float rad) {
	return mat4(1.0,0.0,0.0,0.0,0.0,cos(rad),sin(rad),0.0,0.0,-sin(rad),cos(rad),0.0,0.0,0.0,0.0,1.0);
}
mat4 buildRotateY(float rad) {
	return mat4(cos(rad),0.0,-sin(rad),0.0,0.0,1.0,0.0,0.0,sin(rad),0.0,cos(rad),0.0,0.0,0.0,0.0,1.0);
}
mat4 buildRotateZ(float rad) {
	return mat4(cos(rad),sin(rad),0.0,0.0,-sin(rad),cos(rad),0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0);
}
vec3 checkerboard(vec2 tc) {
	float tileScale = 24.0;
	float tile = mod(floor(tc.x * tileScale) + floor(tc.y * tileScale), 2.0);
	return tile * vec3(1,1,1);
}
vec3 getTextureColor(int index, vec2 tc) {
	
	if (index==1) return (checkerboard(tc)).xyz;
	else if (index==3) return texture(sampMarble, tc).xyz;
	else return vec3(1,.7,.7);
}
Collision intersect_plane_object(Ray r, Object o) {
	
	mat4 local_to_worldT = buildTranslate((o.position).x, (o.position).y, (o.position).z);
	mat4 local_to_worldR =
		buildRotateY(DEG_TO_RAD*o.yrot) * buildRotateX(DEG_TO_RAD*o.xrot) * buildRotateZ(DEG_TO_RAD*o.zrot);
	mat4 local_to_worldTR = local_to_worldT * local_to_worldR;
	mat4 world_to_localTR = inverse(local_to_worldTR);
	mat4 world_to_localR = inverse(local_to_worldR);
	
	vec3 ray_start = (world_to_localTR * vec4(r.start,1.0)).xyz;
	vec3 ray_dir = (world_to_localR * vec4(r.dir,1.0)).xyz;
	
	Collision c;
	c.inside = false;  
c.t = dot((vec3(0,0,0) - ray_start),vec3(0,1,0)) / dot(ray_dir, vec3(0,1,0));
c.p = r.start + c.t * r.dir;
vec3 intersectPoint = ray_start + c.t * ray_dir;
if ((abs(intersectPoint.x) > (((o.mins).x)/2.0)) || (abs(intersectPoint.z) > (((o.mins).z)/2.0))) {
	c.t = -1.0;
		return c;
	}
	
	c.n = vec3(0.0, 1.0, 0.0);
	
	if(ray_dir.y > 0.0) c.n *= -1.0;
c.n = transpose(inverse(mat3(local_to_worldR))) * c.n;
	
	float maxDimension = max(((o.mins).x), ((o.mins).z));
	c.tc = (intersectPoint.xz + ((o.mins).x)/2.0)/maxDimension;
	return c;
}

Collision intersect_box_object(Ray r, Object o) {
	
	mat4 local_to_worldT = buildTranslate((o.position).x, (o.position).y, (o.position).z);
	mat4 local_to_worldR =
		buildRotateY(DEG_TO_RAD*o.yrot) * buildRotateX(DEG_TO_RAD*o.xrot) * buildRotateZ(DEG_TO_RAD*o.zrot);
	mat4 local_to_worldTR = local_to_worldT * local_to_worldR;
	mat4 world_to_localTR = inverse(local_to_worldTR);
	mat4 world_to_localR = inverse(local_to_worldR);
	
	vec3 ray_start = (world_to_localTR * vec4(r.start,1.0)).xyz;
	vec3 ray_dir = (world_to_localR * vec4(r.dir,1.0)).xyz;
vec3 t_min = (o.mins - ray_start) / ray_dir;
	vec3 t_max = (o.maxs - ray_start) / ray_dir;
	vec3 t_minDist = min(t_min, t_max);
	vec3 t_maxDist = max(t_min, t_max);
	float t_near = max(max(t_minDist.x, t_minDist.y), t_minDist.z);
	float t_far = min(min(t_maxDist.x, t_maxDist.y), t_maxDist.z);
	Collision c;
	c.t = t_near;
	c.inside = false;

if(t_near >= t_far || t_far <= 0.0) {
	c.t = -1.0;
		return c;
	}
	float intersection = t_near;
	vec3 boundary = t_minDist;
	
	if( t_near < 0.0) {
	c.t = t_far;
		intersection = t_far;
		boundary = t_maxDist;
		c.inside = true;
	}
	
	int face_index = 0;
	if(intersection == boundary.y) face_index = 1;
	else if(intersection == boundary.z) face_index = 2;
c.n = vec3(0.0);
	c.n[face_index] = 1.0;
	
	if(ray_dir[face_index] > 0.0) c.n *= -1.0;
c.n = transpose(inverse(mat3(local_to_worldR))) * c.n;
	
	c.p = r.start + c.t * r.dir;

	vec3 cp = (world_to_localTR * vec4(c.p,1.0)).xyz;
	
	float totalWidth = (o.maxs).x - (o.mins).x;
	float totalHeight = (o.maxs).y - (o.mins).y;
	float totalDepth = (o.maxs).z - (o.mins).z;
	float maxDimension = max(totalWidth, max(totalHeight, totalDepth));
	
	float rayStrikeX = (cp.x + totalWidth/2.0)/maxDimension;
	float rayStrikeY = (cp.y + totalHeight/2.0)/maxDimension;
	float rayStrikeZ = (cp.z + totalDepth/2.0)/maxDimension;
	if (face_index == 0)
		c.tc = vec2(rayStrikeZ, rayStrikeY);
	else if (face_index == 1)
		c.tc = vec2(rayStrikeZ, rayStrikeX);
	else
		c.tc = vec2(rayStrikeY, rayStrikeX);
		
	return c;
}

Collision intersect_room_box_object(Ray r) {
	
	vec3 t_min = (objects[0].mins - r.start) / r.dir;
	vec3 t_max = (objects[0].maxs - r.start) / r.dir;
	vec3 t1 = min(t_min, t_max);
	vec3 t2 = max(t_min, t_max);
	float t_near = max(max(t1.x,t1.y),t1.z);
	float t_far = min(min(t2.x, t2.y), t2.z);
	Collision c;
	c.t = t_near;
	c.inside = false;

if(t_near >= t_far || t_far <= 0.0) {
	c.t = -1.0;
		return c;
	}
	float intersection = t_near;
	vec3 boundary = t1;
	
	if( t_near < 0.0) {
	c.t = t_far;
		intersection = t_far;
		boundary = t2;
		c.inside = true;
	}
	
	int face_index = 0;
	if(intersection == boundary.y) face_index = 1;
	else if(intersection == boundary.z) face_index = 2;
c.n = vec3(0.0);
	c.n[face_index] = 1.0;
	
	if(r.dir[face_index] > 0.0) c.n *= -1.0;
c.p = r.start + c.t * r.dir;
if (c.n == vec3(1,0,0)) c.face_index = 0;
	else if (c.n == vec3(-1,0,0)) c.face_index = 1;
	else if (c.n == vec3(0,1,0)) c.face_index = 2;
	else if (c.n == vec3(0,-1,0)) c.face_index = 3;
	else if (c.n == vec3(0,0,1)) c.face_index = 4;
	else if (c.n == vec3(0,0,-1)) c.face_index = 5;

	float totalWidth = (objects[0].maxs).x - (objects[0].mins).x;
	float totalHeight = (objects[0].maxs).y - (objects[0].mins).y;
	float totalDepth = (objects[0].maxs).z - (objects[0].mins).z;
	float maxDimension = max(totalWidth, max(totalHeight, totalDepth));
float rayStrikeX = ((c.p).x + totalWidth/2.0)/maxDimension;
	float rayStrikeY = ((c.p).y + totalHeight/2.0)/maxDimension;
	float rayStrikeZ = ((c.p).z + totalDepth/2.0)/maxDimension;
	
	if (c.face_index == 0)
		c.tc = vec2(rayStrikeZ, rayStrikeY);
	else if (c.face_index == 1)
		c.tc = vec2(1.0-rayStrikeZ, rayStrikeY);
	else if (c.face_index == 2)
		c.tc = vec2(rayStrikeX, rayStrikeZ);
	else if (c.face_index == 3)
		c.tc = vec2(rayStrikeX, 1.0-rayStrikeZ);
	else if (c.face_index == 4)
		c.tc = vec2(1.0-rayStrikeX, rayStrikeY);
	else if (c.face_index == 5)
		c.tc = vec2(rayStrikeX, rayStrikeY);
		
	return c;
}

Collision intersect_sphere_object(Ray r, Object o) {
	float qa = dot(r.dir, r.dir);
	float qb = dot(2*r.dir, r.start-o.position);
	float qc = dot(r.start-o.position, r.start-o.position) - o.radius*o.radius;
	
	float qd = qb * qb - 4 * qa * qc;
	Collision c;
	c.inside = false;
	if(qd < 0.0)	
	{	c.t = -1.0;
		return c;
	}
	float t1 = (-qb + sqrt(qd)) / (2.0 * qa);
	float t2 = (-qb - sqrt(qd)) / (2.0 * qa);
	float t_near = min(t1, t2);
	float t_far = max(t1, t2);
	c.t = t_near;
	if(t_far < 0.0)		
	{	c.t = -1.0;
		return c;
	}
	if(t_near < 0.0)	
	{	c.t = t_far;
		c.inside = true;
	}
	c.p = r.start + c.t * r.dir;	
	c.n = normalize(c.p - o.position);	
	if(c.inside)	
	{	c.n *= -1.0;
	}
(c.tc).x = 0.5 + atan(-(c.n).z,(c.n).x)/(2.0*PI);
	(c.tc).y = 0.5 - asin(-(c.n).y)/PI;
	
	return c;
}

Collision get_closest_collision(Ray r) {
	float closest = 10000; 
	Collision closest_collision;
	closest_collision.object_index = -1;
	
	for (int i=0; i<numObjects; i++) {
	Collision c;
		
		if (objects[i].type == 0) {
	c = intersect_room_box_object(r);
			if (c.t <= 0) continue;
		} else if (objects[i].type == 1) {
	c = intersect_sphere_object(r, objects[i]);
			if (c.t <= 0) continue;
		} else if (objects[i].type == 2) {
	c = intersect_box_object(r, objects[i]);
			if (c.t <= 0) continue;
		} else if (objects[i].type == 3) {
	c = intersect_plane_object(r, objects[i]);
			if (c.t <= 0) continue;
		} else continue;
		
		if (c.t < closest) {
	closest = c.t;
			closest_collision = c;
			closest_collision.object_index = i;
	}	}
	return closest_collision;	
}
vec3 ads_phong_lighting(Ray r, Collision c) {
	
	vec4 ambient = worldAmb_ambient + pointLight_ambient * objects[c.object_index].ambient;
vec4 diffuse = vec4(0.0);
	vec4 specular = vec4(0.0);
	
	Ray light_ray;
	light_ray.start = c.p + c.n * 0.01;
	light_ray.dir = normalize(pointLight_position - c.p);
	bool in_shadow = false;
	
	Collision c_shadow = get_closest_collision(light_ray);
	
	if((c_shadow.object_index != -1) && c_shadow.t < length(pointLight_position - c.p)) {
	in_shadow = true;
	}
	
	if (in_shadow == false)	
	{	
		vec3 light_dir = normalize(pointLight_position - c.p);
		vec3 light_ref = normalize(reflect(-light_dir, c.n));
		float cos_theta = dot(light_dir, c.n);
		float cos_phi = dot(normalize(-r.dir), light_ref);
		diffuse = pointLight_diffuse * objects[c.object_index].diffuse * max(cos_theta, 0.0);
		specular = pointLight_specular
			* objects[c.object_index].specular
			* pow(max(cos_phi, 0.0), objects[c.object_index].shininess);
	}
	vec4 phong_color = ambient + diffuse + specular;
	return phong_color.rgb;
}

void push(Ray r, int depth, int type) {
	if(stack_pointer >= stack_size-1)  return;
	Stack_Element element;
	element = null_stack_element;
	element.type = type;
	element.depth = depth;
	element.phase = 1;
	element.ray = r;
	stack_pointer++;
	stack[stack_pointer] = element;
}
Stack_Element pop() {
	
	Stack_Element top_stack_element = stack[stack_pointer];
stack[stack_pointer] = null_stack_element;
	stack_pointer--;
	return top_stack_element;
}
vec3 rgb2ryb(vec3 rgb) {
	float white = min(rgb.r, min(rgb.g, rgb.b));
	float black = min((1-rgb.r), min((1-rgb.g), (1-rgb.b)));
	vec3 rgbWhiteRemoved = rgb - white;
	vec3 buildRYB = vec3(
		rgbWhiteRemoved.r - min(rgbWhiteRemoved.r, rgbWhiteRemoved.g),
		(rgbWhiteRemoved.g + min(rgbWhiteRemoved.r, rgbWhiteRemoved.g)) / 2.0,
		(rgbWhiteRemoved.b + rgbWhiteRemoved.g - min(rgbWhiteRemoved.r, rgbWhiteRemoved.g)) / 2.0);
	float normalizeFactor = max(buildRYB.x, max(buildRYB.y, buildRYB.z))
						/ max(rgbWhiteRemoved.r, max(rgbWhiteRemoved.g, rgbWhiteRemoved.b));
	buildRYB /= normalizeFactor;
	buildRYB += black;
	return buildRYB;
}
vec3 ryb2rgb(vec3 ryb) {
	float white = min(ryb.x, min(ryb.y, ryb.z));
	float black = min((1-ryb.x),min((1-ryb.y),(1-ryb.z)));
	vec3 rybWhiteRemoved = ryb - white;
	vec3 buildRGB = vec3(
		rybWhiteRemoved.x + rybWhiteRemoved.y - min(rybWhiteRemoved.y, rybWhiteRemoved.z),
		rybWhiteRemoved.y + 2.0 * min(rybWhiteRemoved.y, rybWhiteRemoved.z),
		2.0 * rybWhiteRemoved.z - min(rybWhiteRemoved.y, rybWhiteRemoved.z));
	float normalizeFactor = max(buildRGB.r, max(buildRGB.g, buildRGB.b))
						/ max(rybWhiteRemoved.x, max(rybWhiteRemoved.y, rybWhiteRemoved.z));
	buildRGB /= normalizeFactor;
	buildRGB += black;
	return buildRGB;
}
void process_stack_element(int index) {
if(popped_stack_element != null_stack_element) {
	if(popped_stack_element.type == RAY_TYPE_REFLECTION)
			stack[index].reflected_color = popped_stack_element.final_color;
		else if(popped_stack_element.type == RAY_TYPE_REFRACTION) {
	if (popped_stack_element.collision.object_index == 0)
				stack[index].refracted_color = vec3(-1,-1,-1);
			else
				stack[index].refracted_color = popped_stack_element.final_color;
		}
		popped_stack_element = null_stack_element;
	}
	Ray r = stack[index].ray;
	Collision c = stack[index].collision;
	
	switch (stack[index].phase) {
	
		
		
		case 1:
			c = get_closest_collision(r);	
			if (c.object_index != -1)		
				stack[index].collision = c;
			break;
		
		
		
		case 2:
			stack[index].phong_color = ads_phong_lighting(r, c);
			break;
		
		
		
		case 3:
			
			if(stack[index].depth < max_depth) {
	if (objects[c.object_index].isReflective) {
	Ray reflected_ray;
					reflected_ray.start = c.p + c.n * 0.001;
					reflected_ray.dir = reflect(r.dir, c.n);
				
					
					push(reflected_ray, stack[index].depth+1, RAY_TYPE_REFLECTION);
			}	}
			break;
		
		
		
		case 4:
			
			if(stack[index].depth < max_depth) {
	if (objects[c.object_index].isRefractive) {
	Ray refracted_ray;
					refracted_ray.start = c.p - c.n * 0.001;
					float refraction_ratio = 1.0 / objects[c.object_index].IOR;
					if (c.inside) refraction_ratio = 1.0 / refraction_ratio;
					refracted_ray.dir = refract(r.dir, c.n, refraction_ratio);
			
					
					push(refracted_ray, stack[index].depth+1, RAY_TYPE_REFRACTION);
			}	}
			break;
		
		
		
		case 5:
			if (c.object_index > 0)  
			{	
				vec3 texColor = vec3(0.0);
				if (objects[c.object_index].hasTexture)
					texColor = getTextureColor(c.object_index, c.tc);
				
				
				vec3 objColor = vec3(0.0);
				if (objects[c.object_index].hasColor)
					objColor = objects[c.object_index].color;
				
				
				vec3 reflected_color = stack[index].reflected_color;
				vec3 refracted_color = stack[index].refracted_color;
				
				
				vec3 mixed_color = objColor + texColor;
				if ((objects[c.object_index].isReflective) && (stack[index].depth<max_depth))
					mixed_color = mix(mixed_color, reflected_color, objects[c.object_index].reflectivity);
				if ((objects[c.object_index].isRefractive) && (stack[index].refracted_color.x != -1)) {
	vec3 mxRYB = rgb2ryb(mixed_color);
					vec3 rfRYB = rgb2ryb(refracted_color);
					vec3 blend = mxRYB + rfRYB;
					mixed_color = ryb2rgb(blend);
				}
				stack[index].final_color = mixed_color;
			}
			if (c.object_index == 0) {
	vec3 lightFactor = vec3(1.0);
				if (objects[c.object_index].isReflective)
					lightFactor = stack[index].phong_color;
				if (objects[c.object_index].hasColor)
					stack[index].final_color = lightFactor * objects[c.object_index].color;
				else
				{	if (c.face_index == 0)
						stack[index].final_color = lightFactor * getTextureColor(5, c.tc);
					else if (c.face_index == 1)
						stack[index].final_color = lightFactor * getTextureColor(6, c.tc);
					else if (c.face_index == 2)
						stack[index].final_color = lightFactor * getTextureColor(7, c.tc);
					else if (c.face_index == 3)
						stack[index].final_color = lightFactor * getTextureColor(8, c.tc);
					else if (c.face_index == 4)
						stack[index].final_color = lightFactor * getTextureColor(9, c.tc);
					else if (c.face_index == 5)
						stack[index].final_color = lightFactor * getTextureColor(10, c.tc);
			}	}
			break;
		
		
		
		case 6: { popped_stack_element = pop(); return; }
	}
	stack[index].phase++;
	return;	
}
vec3 raytrace(Ray r) {
	
	push(r, 0, RAY_TYPE_REFLECTION);
	
	while (stack_pointer >= 0) {
	int element_index = stack_pointer;	
		process_stack_element(element_index);	
	}
	
	return popped_stack_element.final_color;
}

void main() {
	int width = int(gl_NumWorkGroups.x);
	int height = int(gl_NumWorkGroups.y);
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	
	float x_pixel = 2.0 * pixel.x/width - 1.0;
	float y_pixel = 2.0 * pixel.y/height - 1.0;
	Ray world_ray;
	world_ray.start = vec3(0.0, 0.0, camera_pos);
	vec4 world_ray_end = vec4(x_pixel, y_pixel, camera_pos-1.0, 1.0);
	world_ray.dir = normalize(world_ray_end.xyz - world_ray.start);
	
	vec3 color = raytrace(world_ray);
	imageStore(output_texture, pixel, vec4(color,1.0));
}