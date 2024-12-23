#version 430
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

layout(local_size_x = 1) in;
layout(binding = 0, rgba8) uniform image2D output_texture;
layout(binding = 1) uniform sampler2D sampMarble;
layout(binding = 2, std430) buffer vertBuffer{float  triVertsBuffer[];};
layout(binding = 3, std430) buffer texBuffer{float triTCsBuffer[];};
layout(binding = 4, std430) buffer normBuffer{float triNormsBuffer[];};
layout(binding = 5, std430) buffer nodeBuffer{float bvhNodesBuffer[];};

struct TriV3
{
	vec3 p0;
	vec3 p1;
	vec3 p2;
};

struct TriT2
{
	vec2 p0;
	vec2 p1;
	vec2 p2;
};

struct TriN3
{
	vec3 p0;
	vec3 p1;
	vec3 p2;
};

struct Object
{
	int type;
	float radius;
	vec3 mins;
	vec3 maxs;
	float xrot;
	float yrot;
	float zrot;
	vec3 position;
	bool hasColor;
	bool hasTexture;
	bool isReflective;
	bool isTransparent;
	vec3 color;
	float reflectivity;
	float refractivity;
	float IOR;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
	TriV3 triVerts;
	TriT2 triTCs;
	TriN3 triNorms;
	int firstTriIdx;
	int triCount;
	int bvhRootIdx;
};

struct Ray
{
	vec3 start;
	vec3 dir;
};

struct Collision
{
	float t;
	vec3 p;
	vec3 n;
	bool inside;
	int object_index;
	vec2 tc;
	int face_index;
};

struct Stack_Element
{
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

const int TYPE_ROOM = 0;
const int TYPE_SPHERE = 1;
const int TYPE_BOX = 2;
const int TYPE_PLANE = 3;
const int TYPE_TRIANGLE = 4;
const int TYPE_MESH = 5;
const int TYPE_BVH = 6;
const int max_depth = 4;
const int stack_size = 100;
const int RAY_TYPE_REFLECTION = 1;
const int RAY_TYPE_REFRACTION = 2;
const int BVH_QUEUE_SIZE = 256;
const float PI = 3.14159265358;
const float DEG_TO_RAD = PI / 180.0;
const TriT2 TRITEXCS_ZERO = {vec2(0), vec2(0), vec2(0)};
const TriN3 TRINORMS_ZERO = {vec3(0), vec3(0), vec3(0)};
const TriV3 TRIVERTS_ZERO = {vec3(0), vec3(0), vec3(0)};

int stack_pointer = -1;
int numObjects = 9;
int bvh_queue[BVH_QUEUE_SIZE];
int bvh_queue_front;
int bvh_queue_end;
float camera_pos = 5.0;
vec3 pointLight_position = vec3(-18.0, 6.0, 3.5);
vec4 worldAmb_ambient = vec4(0.3, 0.3, 0.3, 1.0);
vec4 pointLight_ambient = vec4(0.2, 0.2, 0.2, 1.0);
vec4 pointLight_diffuse = vec4(0.7, 0.7, 0.7, 1.0);
vec4 pointLight_specular = vec4(1.0, 1.0, 1.0, 1.0);

Ray null_ray = {vec3(0.0), vec3(0.0)};

Collision null_collision = {-1.0, vec3(0.0), vec3(0.0), false, -1, vec2(0.0, 0.0), -1};
Collision intersect_triangle(Ray r, TriV3 verts, TriT2 tcs, TriN3 norms);
Collision intersect_triangle_object(Ray r, Object o);
Collision intersect_triangle_from_buffer(Ray r, int index, mat4 local_to_worldTR, mat4 norm_local_to_worldTR);
Collision intersect_plane_object(Ray r, Object o);
Collision intersect_box_object(Ray r, Object o);
Collision intersect_room_box_object(Ray r);
Collision intersect_sphere_object(Ray r, Object o);
Collision get_closest_collision(Ray r);

Stack_Element null_stack_element = {0, -1, -1, vec3(0), vec3(0), vec3(0), vec3(0), null_ray, null_collision};
Stack_Element stack[stack_size];
Stack_Element popped_stack_element;
Stack_Element pop();

float det_3x3(vec3 col1, vec3 col2, vec3 col3);

mat4 buildTranslate(float x, float y, float z);
mat4 buildRotateX(float rad);
mat4 buildRotateY(float rad);
mat4 buildRotateZ(float rad);

vec3 checkerboard(vec2 tc);
vec3 getTextureColor(int index, vec2 tc);
vec3 ads_phong_lighting(Ray r, Collision c);
vec3 raytrace(Ray r);

void push(Ray r, int depth, int type);
void process_stack_element(int index);
void main();

void clear_bvh_queue() { 
	bvh_queue_front = bvh_queue_end = 0; 
}

bool is_bvh_queue_empty() { 
	return bvh_queue_front == bvh_queue_end;
} 

void enqueue_bvh_node(int index) {
	bvh_queue[bvh_queue_end] = index;
	bvh_queue_end = (bvh_queue_end + 1) % BVH_QUEUE_SIZE;
}

int dequeue_bvh_node() {
	int index = bvh_queue[bvh_queue_front];
	bvh_queue_front = (bvh_queue_front + 1) % BVH_QUEUE_SIZE; 
	return index;
}

float intersect_bounding_volume(Ray r, int nodeStartIdx, mat4 world_to_localR,mat4 world_to_localTR)
{ 
	vec3 mins = vec3(bvhNodesBuffer[nodeStartIdx+1], bvhNodesBuffer[nodeStartIdx+3], bvhNodesBuffer[nodeStartIdx+5]);
	vec3 maxs = vec3(bvhNodesBuffer[nodeStartIdx+2], bvhNodesBuffer[nodeStartIdx+4], bvhNodesBuffer[nodeStartIdx+6]);
	// Convert the world-space ray to the box local space: 
	vec3 ray_start = (world_to_localTR * vec4(r.start, 1.0)).xyz;
	vec3 ray_dir = (world_to_localR * vec4(r.dir, 1.0)).xyz;
	// Calculate the box world mins and maxs (as described in Chapter 16): 
	vec3 t_min= (mins - ray_start) / ray_dir;
	vec3 t_max = (maxs - ray_start) /ray_dir; 
	vec3 t_minDist = min(t_min, t_max); 
	vec3 t_maxDist = max(t_min, t_max);
	float t_near = max(max(t_minDist.x, t_minDist.y), t_minDist.z); 
	float t_far = min(min(t_maxDist.x, t_maxDist.y), t_maxDist.z);
	//
	if(t_near >= t_far || t_far <= 0.0 || t_near < 0.0) {return -1.0;}
	return t_near;	
}

Collision intersect_bvh_object(Ray r, Object o)
{
	const int LEFT_CHILD_OFFSET = 7;
	const int RIGHT_CHILD_OFFSET = 8;
	float closest = FLT_MAX;
	Collision c, closest_collision;
	closest_collision.t = -1;

	//build translation and rotation matrices for applying to bounding box
	mat4 local_to_worldT = buildTranslate((o.position).x, (o.position).y, (o.position).z); 
	mat4 local_to_worldR = 
		buildRotateY( DEG_TO_RAD*o.yrot) * 
		buildRotateX(DEG_TO_RAD*o.xrot) * 
		buildRotateZ(DEG_TO_RAD*o.zrot);

	mat4 local_to_worldTR = local_to_worldT * local_to_worldR;
	mat4 norm_local_to_worldTR = transpose(inverse(local_to_worldTR)); 
	mat4 world_to_localR = inverse(local_to_worldR);
	mat4 world_to_localTR = inverse(local_to_worldTR);

	// initialize the queue, and enqueue the root node of the BVH
	clear_bvh_queue();
	enqueue_bvh_node(0);

	//iterate through the queue of BVH nodes until it is empty
	while (!is_bvh_queue_empty())
	{
		int nodeStartIdx = o.bvhRootIdx + dequeue_bvh_node();
		int triangleIdx = int(bvhNodesBuffer[nodeStartIdx]);
	
		// test to see if the triangle is a leaf node
		if (triangleIdx >= 0)
		{
			c = intersect_triangle_from_buffer(r, o.firstTriIdx + triangleIdx, local_to_worldTR, norm_local_to_worldTR);
			if (c.t> 0 && c.t < closest)
			{
				closest = c.t;
				closest_collision = c;
			}
		}
		else // it is an inner node
		{
			float t = intersect_bounding_volume(r, nodeStartIdx, world_to_localR, world_to_localTR);
			if (t> 0 && t <= closest)
			{ 
				//if there is a left child, add it to the queue
				int leftChildIdx = int(bvhNodesBuffer[nodeStartIdx + LEFT_CHILD_OFFSET]); 
				if (leftChildIdx >= 0) { enqueue_bvh_node(leftChildIdx); }
				
				//if there is a right child, add it to the queue
				int rightChildIdx = int(bvhNodesBuffer[nodeStartIdx + RIGHT_CHILD_OFFSET]);
				if (rightChildIdx >= 0) { enqueue_bvh_node(rightChildIdx);}
		}}}
		return closest_collision;
}

Object[] objects = {
	{
		TYPE_ROOM,					/*int type;*/	
		0.0,						/*float radius;*/	
		vec3(-20, -20, -20),		/*vec3 mins;*/	
		vec3(20, 20, 20),			/*vec3 maxs;*/	
		0,							/*float xrot;*/	
		0,							/*float yrot;*/	
		0,							/*float zrot;*/	
		vec3(0),					/*vec3 position;*/	
		true,						/*bool hasColor;*/	
		false,						/*bool hasTexture;*/	
		true,						/*bool isReflective;*/	
		false,						/*bool isTransparent;*/	
		vec3(0.25, 0.5, 0.8),		/*vec3 color;*/	
		0,							/*float reflectivity;*/	
		0,							/*float refractivity;*/	
		0,							/*float IOR;*/	
		vec4(0.2, 0.2, 0.2, 1.0),	/*vec4 ambient;*/	
		vec4(0.9, 0.9, 0.9, 1.0),	/*vec4 diffuse;*/	
		vec4(1, 1, 1, 1),			/*vec4 specular;*/	
		50.0,						/*float shininess;*/	
		TRIVERTS_ZERO,				/*TriV3 triVerts;*/	
		TRITEXCS_ZERO,				/*TriT2 triTCs;*/	
		TRINORMS_ZERO				/*TriN3 triNorms;*/	
		, 
		0,  // 	int firstTriIdx;
		0,  // 	int triCount;
		0   // 	int bvhRootIdx;
	},
	// floor
	{
		TYPE_PLANE, 0.0, vec3(12, 0, 16), vec3(0), 0.0, 0.0, 0.0, vec3(0.0, -1.0, -2.0),
		false, true, false, false, vec3(0), 0.0, 0.0, 0.0,
		vec4(0.2, 0.2, 0.2, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(1, 1, 1, 1), 50.0,
		TRIVERTS_ZERO, TRITEXCS_ZERO, TRINORMS_ZERO, 0, 0,0
	},
	// sphere
	 {
	 	TYPE_SPHERE, 1.2, vec3(0), vec3(0), 0, 0, 0, vec3(0.0, 2.0, 0.0),
	 	false, false, true, true, vec3(0, 0, 0), 0.8, 0.8, 1.02,
	 	vec4(0.5, 0.5, 0.5, 1.0), vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), 50.0,
	 	TRIVERTS_ZERO, TRITEXCS_ZERO, TRINORMS_ZERO, 0, 0, 0
	 },
	 	//high-poly textured dolphin
	{
		TYPE_BVH, 0.0, vec3(0), vec3(0), 0.0, 40.0, 0.0, vec3(0.0, 0.0, 0.0),
		false, true, false, false, vec3(0), 0.0, 0.0, 0.0,
		vec4(0.5, 0.5, 0.5, 1.0), vec4(.5,.5,.5,1), vec4(.5,.5,.5,1), 50.0,
		TRIVERTS_ZERO, TRITEXCS_ZERO, TRINORMS_ZERO, 0,0,0
	},
	
	// transparent blue-green pyramid
	 {
 		TYPE_BVH, 0.0, vec3(0), vec3(0), 0.0, 60.0, 0.0, vec3(-2.0, 0.0, 1.0), 
 		true, false, false, true, vec3(0,0.7,0.5), 0.0, 0.3, 1.01,
 		vec4(0.5, 0.5, 0.5, 1.0), vec4(.5, .5, .5,1), vec4(.5, 5, 5,1), 100.0,
 		TRIVERTS_ZERO, TRITEXCS_ZERO, TRINORMS_ZERO, 5664, 0, 101943
	 },
	 	// transparent and reflective red icosahedron
	 {
	 	TYPE_BVH, 0.0, vec3(0), vec3(0), 60.0, 40.0, 10.0, vec3(1.5, 0.0, 2.5), 
	 	true, false, true, true, vec3(1.0,0,0), 0.5, 0.3, 1.0,
	 	vec4(0.5, 0.5, 0.5, 1.0), vec4(.5,.5,.5,1), vec4(.5, .5, .5,1), 50.0,
	 	TRIVERTS_ZERO, TRITEXCS_ZERO, TRINORMS_ZERO, 5670, 0, 102042
	 },
	  // icosahedron.obj
	  {
	  	TYPE_MESH, 0.0, vec3(0), vec3(0), 60.0, 0.0, 0.0, vec3(-2, 0.0, 0.75),
	  	false, true, false, true, vec3(0, 0.7, 0.5), 1.0, 0.3, 1.05,
	  	vec4(0.5, 0.5, 0.5, 1.0), vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), 50.0,
	  	TRIVERTS_ZERO, TRITEXCS_ZERO, TRINORMS_ZERO, 5690, 0, 102393
	  },
	// pyr.obj
	 {
	 	TYPE_MESH, 0.0, vec3(0), vec3(0), 0.0, 45.0, 0.0, vec3(1.0, 0.0, 1.5),
	 	true, false, false, false, vec3(0, 0.7, 0.5), 0.0, 0.0, 1.0,
	 	vec4(0.5, 0.5, 0.5, 1.0), vec4(0.5, 0.5, 0.5, 1), vec4(1, 1, 1, 1), 50.0,
	 	TRIVERTS_ZERO, TRITEXCS_ZERO, TRINORMS_ZERO, 5710, 0, 102744
	 }

};

Collision intersect_triangle_from_buffer(Ray r, int index, mat4 local_to_worldTR, mat4 norm_local_to_worldTR)
{
	TriV3 verts; 
	TriT2 tcs;
	TriN3 norms;
	Collision c;
	//copy vertices from SSBO (3 vertices in this triangle, each with 3 fields (X,Y,Z) = 9 fields)
	verts.p0.x = triVertsBuffer[index*9+0];
	verts.p0.y = triVertsBuffer[index*9+1]; 
	verts.p0.z = triVertsBuffer[index*9+2]; 
	verts.p1.x = triVertsBuffer[index*9+3]; 
	verts.p1.y = triVertsBuffer[index*9+4]; 
	verts.p1.z = triVertsBuffer[index*9+5]; 
	verts.p2.x = triVertsBuffer[index*9+6]; 
	verts.p2.y = triVertsBuffer[index*9+7]; 
	verts.p2.z = triVertsBuffer[index*9+8];

	//apply transformations to vertices
	verts.p0 = (local_to_worldTR * vec4(verts.p0, 1.0)).xyz; 
	verts.p1 = (local_to_worldTR * vec4(verts.p1, 1.0)).xyz;
	verts.p2 = (local_to_worldTR * vec4(verts.p2, 1.0)).xyz;

	// copy tex coords from SSBO (3 tex coords in this triangle, each with 2 fields (S,T) = 6 fields) 
	tcs.p0.x = triTCsBuffer[index*6+0];
	tcs.p0.y = triTCsBuffer[index*6+1]; 
	tcs.p1.x = triTCsBuffer[index*6+2]; 
	tcs.p1.y = triTCsBuffer[index*6+3];
	tcs.p2.x = triTCsBuffer[index*6+4]; 
	tcs.p2.y = triTCsBuffer[index*6+5];
	
	// copy normals from SSBO (3 normals in this triangle, each with 3 fields (X,Y,Z) = 9 fields) 
	norms.p0.x = triNormsBuffer[index* 9+0];
	norms.p0.y = triNormsBuffer[index*9+1]; 
	norms.p0.z = triNormsBuffer[index*9+2];
	norms.p1.x = triNormsBuffer[index*9+3]; 
	norms.p1.y = triNormsBuffer[index*9+4]; 
	norms.p1.z = triNormsBuffer[index*9+5]; 
	norms.p2.x = triNormsBuffer[index*9+6]; 
	norms.p2.y = triNormsBuffer[index*9+7]; 
	norms.p2.z = triNormsBuffer[index*9+8];

	// apply transformations to normals
	norms.p0 = (norm_local_to_worldTR * vec4(norms.p0, 1.0)).xyz;
	norms.p1 = (norm_local_to_worldTR * vec4(norms.p1, 1.0)).xyz; 
	norms.p2 = (norm_local_to_worldTR * vec4(norms.p2, 1.0)).xyz; 
	
	// test for intersection
	c = intersect_triangle(r, verts, tcs, norms);
	return c;
}

Collision intersect_mesh_object(Ray r, Object o)
{
	float closest = FLT_MAX;
	Collision c, closest_collision;
	closest_collision.t = -1;

	// build rotation and translation matrices for applying to triangles
	mat4 local_to_worldT = buildTranslate((o.position).x, (o.position).y, (o.position).z); 
	mat4 local_to_worldR = 
		buildRotateY(DEG_TO_RAD*o.yrot) *
		buildRotateX(DEG_TO_RAD*o.xrot)	*
		buildRotateZ(DEG_TO_RAD*o.zrot);

	mat4 local_to_worldTR = local_to_worldT * local_to_worldR;
	mat4 norm_local_to_worldTR = transpose(inverse(local_to_worldTR));

	// iterate over all triangles in the mesh
	for (int i = 0; i < o.triCount; i++)
	{
		c = intersect_triangle_from_buffer(r, o.firstTriIdx +i, local_to_worldTR, norm_local_to_worldTR);
		if(c.t > 0 && c.t < closest)
		{
			closest = c.t;
			closest_collision = c;
		}
	}
	return closest_collision;
}


float det_3x3(vec3 col1, vec3 col2, vec3 col3)
{
	return dot(-cross(col1, col3), col2);
}

Collision intersect_triangle(Ray r, TriV3 verts, TriT2 tcs, TriN3 norms)
{
	Collision c;
	c.t = -1.0; 
	vec3 E1 = verts.p1 - verts.p0;
	vec3 E2 = verts.p2 - verts.p0;
	vec3 T = r.start - verts.p0;
	float det = det_3x3(-r.dir, E1, E2);
	if (det < 1e-9) { return c; }
	float coeff = 1.0 / det;
	float u = coeff * det_3x3(-r.dir, T, E2);
	if (u < 0.0 || u > 1.0) { return c; }
	float v = coeff * det_3x3(-r.dir, E1, T);
	if (v < 0.0 || u + v > 1.0) { return c; }
	c.t = coeff * det_3x3(T, E1, E2);
	if (c.t < 0.01) { c.t = -1.0; return c; }
	c.p = r.start + r.dir * c.t;
	c.n = normalize((1.0 - u - v) * norms.p0 + u * norms.p1 + v * norms.p2);
	c.tc = (1.0 - u - v) * tcs.p0 + u * tcs.p1 + v * tcs.p2;
	c.inside = dot(c.n, r.dir) > 0.0;
	return c;
}

mat4 buildTranslate(float x, float y, float z)
{
	return mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, x, y, z, 1.0);
}

mat4 buildRotateX(float rad)
{
	return mat4(1.0, 0.0, 0.0, 0.0, 0.0, cos(rad), sin(rad), 0.0, 0.0, -sin(rad), cos(rad), 0.0, 0.0, 0.0, 0.0, 1.0);
}

mat4 buildRotateY(float rad)
{
	return mat4(cos(rad), 0.0, -sin(rad), 0.0, 0.0, 1.0, 0.0, 0.0, sin(rad), 0.0, cos(rad), 0.0, 0.0, 0.0, 0.0, 1.0);
}

mat4 buildRotateZ(float rad)
{
	return mat4(cos(rad), sin(rad), 0.0, 0.0, -sin(rad), cos(rad), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
}

vec3 checkerboard(vec2 tc)
{
	float tileScale = 24.0;
	float tile = mod(floor(tc.x * tileScale) + floor(tc.y * tileScale), 2.0);
	return tile * vec3(1, 1, 1);
}

vec3 getTextureColor(int index, vec2 tc)
{
	if (index == 1) return (checkerboard(tc)).xyz;
	else if (index >= 3) return texture(sampMarble, tc).xyz;
	else return vec3(1, .7, .7);
}

Collision intersect_triangle_object(Ray r, Object o)
{ 
	mat4 local_to_worldT = buildTranslate((o.position).x, (o.position).y, (o.position).z);
	mat4 local_to_worldR = buildRotateY(DEG_TO_RAD * o.yrot) * buildRotateX(DEG_TO_RAD * o.xrot) * buildRotateZ(DEG_TO_RAD * o.zrot);
	mat4 local_to_worldTR = local_to_worldT * local_to_worldR;
	mat4 norm_local_to_worldTR = transpose(inverse(local_to_worldTR));
	vec3 worldVerts0 = (local_to_worldTR * vec4(o.triVerts.p0, 1.0)).xyz;
	vec3 worldVerts1 = (local_to_worldTR * vec4(o.triVerts.p1, 1.0)).xyz;
	vec3 worldVerts2 = (local_to_worldTR * vec4(o.triVerts.p2, 1.0)).xyz;
	TriV3 worldVerts = {worldVerts0, worldVerts1, worldVerts2};
	vec3 worldNorms0 = (norm_local_to_worldTR * vec4(o.triNorms.p0, 1.0)).xyz;
	vec3 worldNorms1 = (norm_local_to_worldTR * vec4(o.triNorms.p1, 1.0)).xyz;
	vec3 worldNorms2 = (norm_local_to_worldTR * vec4(o.triNorms.p2, 1.0)).xyz;
	TriN3 worldNorms = {worldNorms0, worldNorms1, worldNorms2};
	return intersect_triangle(r, worldVerts, o.triTCs, worldNorms);
}

Collision intersect_plane_object(Ray r, Object o)
{
	mat4 local_to_worldT = buildTranslate((o.position).x, (o.position).y, (o.position).z);
	mat4 local_to_worldR = buildRotateY(DEG_TO_RAD * o.yrot) * buildRotateX(DEG_TO_RAD * o.xrot) * buildRotateZ(DEG_TO_RAD * o.zrot);
	mat4 local_to_worldTR = local_to_worldT * local_to_worldR;
	mat4 world_to_localTR = inverse(local_to_worldTR);
	mat4 world_to_localR = inverse(local_to_worldR);
	vec3 ray_start = (world_to_localTR * vec4(r.start, 1.0)).xyz;
	vec3 ray_dir = (world_to_localR * vec4(r.dir, 1.0)).xyz;
	Collision c;
	c.inside = false;
	c.t = dot((vec3(0, 0, 0) - ray_start), vec3(0, 1, 0)) / dot(ray_dir, vec3(0, 1, 0));
	c.p = r.start + c.t * r.dir;
	vec3 intersectPoint = ray_start + c.t * ray_dir;
	if ((abs(intersectPoint.x) > (((o.mins).x) / 2.0)) || (abs(intersectPoint.z) > (((o.mins).z) / 2.0)))
	{
		c.t = -1.0;
		return c;
	}
	c.n = vec3(0.0, 1.0, 0.0);
	if (ray_dir.y > 0.0) c.n *= -1.0;
	c.n = transpose(inverse(mat3(local_to_worldR))) * c.n;
	float maxDimension = max(((o.mins).x), ((o.mins).z));
	c.tc = (intersectPoint.xz + ((o.mins).x) / 2.0) / maxDimension;
	return c;
}

Collision intersect_box_object(Ray r, Object o)
{
	mat4 local_to_worldT = buildTranslate((o.position).x, (o.position).y, (o.position).z);
	mat4 local_to_worldR = buildRotateY(DEG_TO_RAD * o.yrot) * buildRotateX(DEG_TO_RAD * o.xrot) * buildRotateZ(DEG_TO_RAD * o.zrot);
	mat4 local_to_worldTR = local_to_worldT * local_to_worldR;
	mat4 world_to_localTR = inverse(local_to_worldTR);
	mat4 world_to_localR = inverse(local_to_worldR);
	vec3 ray_start = (world_to_localTR * vec4(r.start, 1.0)).xyz;
	vec3 ray_dir = (world_to_localR * vec4(r.dir, 1.0)).xyz;
	vec3 t_min = (o.mins - ray_start) / ray_dir;
	vec3 t_max = (o.maxs - ray_start) / ray_dir;
	vec3 t_minDist = min(t_min, t_max);
	vec3 t_maxDist = max(t_min, t_max);
	float t_near = max(max(t_minDist.x, t_minDist.y), t_minDist.z);
	float t_far = min(min(t_maxDist.x, t_maxDist.y), t_maxDist.z);
	Collision c;
	c.t = t_near;
	c.inside = false;
	if (t_near >= t_far || t_far <= 0.0){c.t = -1.0;return c;}
	float intersect_distance = t_near;
	vec3 plane_intersect_distances = t_minDist;
	if (t_near < 0.0)
	{
		c.t = t_far;
		intersect_distance = t_far;
		plane_intersect_distances = t_maxDist;
		c.inside = true;
	}
	int face_index = 0;
	if (intersect_distance == plane_intersect_distances.y) face_index = 1;
	else if (intersect_distance == plane_intersect_distances.z) face_index = 2;
	c.n = vec3(0.0);
	c.n[face_index] = 1.0;
	if (ray_dir[face_index] > 0.0) c.n *= -1.0;
	c.n = transpose(inverse(mat3(local_to_worldR))) * c.n;
	c.p = r.start + c.t * r.dir;
	vec3 cp = (world_to_localTR * vec4(c.p, 1.0)).xyz;
	float totalWidth = (o.maxs).x - (o.mins).x;
	float totalHeight = (o.maxs).y - (o.mins).y;
	float totalDepth = (o.maxs).z - (o.mins).z;
	float maxDimension = max(totalWidth, max(totalHeight, totalDepth));
	float rayStrikeX = (cp.x + totalWidth / 2.0) / maxDimension;
	float rayStrikeY = (cp.y + totalHeight / 2.0) / maxDimension;
	float rayStrikeZ = (cp.z + totalDepth / 2.0) / maxDimension;
	if (face_index == 0) c.tc = vec2(rayStrikeZ, rayStrikeY);
	else if (face_index == 1) c.tc = vec2(rayStrikeZ, rayStrikeX);
	else c.tc = vec2(rayStrikeY, rayStrikeX);
	return c;
}

Collision intersect_room_box_object(Ray r)
{
	vec3 t_min = (objects[0].mins - r.start) / r.dir;
	vec3 t_max = (objects[0].maxs - r.start) / r.dir;
	vec3 t1 = min(t_min, t_max);
	vec3 t2 = max(t_min, t_max);
	float t_near = max(max(t1.x, t1.y), t1.z);
	float t_far = min(min(t2.x, t2.y), t2.z);
	Collision c;
	c.t = t_near;
	c.inside = false;
	if (t_near >= t_far || t_far <= 0.0){c.t = -1.0;return c;}
	float intersection = t_near;
	vec3 boundary = t1;
	if (t_near < 0.0)
	{
		c.t = t_far;
		intersection = t_far;
		boundary = t2;
		c.inside = true;
	}
	int face_index = 0;
	if (intersection == boundary.y) face_index = 1;
	else if (intersection == boundary.z) face_index = 2;
	c.n = vec3(0.0);
	c.n[face_index] = 1.0;
	if (r.dir[face_index] > 0.0) c.n *= -1.0;
	c.p = r.start + c.t * r.dir;
	if (c.n == vec3(1, 0, 0)) c.face_index = 0;
	else if (c.n == vec3(-1, 0, 0)) c.face_index = 1;
	else if (c.n == vec3(0, 1, 0)) c.face_index = 2;
	else if (c.n == vec3(0, -1, 0)) c.face_index = 3;
	else if (c.n == vec3(0, 0, 1)) c.face_index = 4;
	else if (c.n == vec3(0, 0, -1)) c.face_index = 5;
	float totalWidth = (objects[0].maxs).x - (objects[0].mins).x;
	float totalHeight = (objects[0].maxs).y - (objects[0].mins).y;
	float totalDepth = (objects[0].maxs).z - (objects[0].mins).z;
	float maxDimension = max(totalWidth, max(totalHeight, totalDepth));
	float rayStrikeX = ((c.p).x + totalWidth / 2.0) / maxDimension;
	float rayStrikeY = ((c.p).y + totalHeight / 2.0) / maxDimension;
	float rayStrikeZ = ((c.p).z + totalDepth / 2.0) / maxDimension;
	if (c.face_index == 0) c.tc = vec2(rayStrikeZ, rayStrikeY);
	else if (c.face_index == 1) c.tc = vec2(1.0 - rayStrikeZ, rayStrikeY);
	else if (c.face_index == 2) c.tc = vec2(rayStrikeX, rayStrikeZ);
	else if (c.face_index == 3) c.tc = vec2(rayStrikeX, 1.0 - rayStrikeZ);
	else if (c.face_index == 4) c.tc = vec2(1.0 - rayStrikeX, rayStrikeY);
	else if (c.face_index == 5) c.tc = vec2(rayStrikeX, rayStrikeY);
	return c;
}

Collision intersect_sphere_object(Ray r, Object o)
{
	float qa = dot(r.dir, r.dir);
	float qb = dot(2 * r.dir, r.start - o.position);
	float qc = dot(r.start - o.position, r.start - o.position) - o.radius * o.radius;
	float qd = qb * qb - 4 * qa * qc;
	Collision c;
	c.inside = false;
	if (qd < 0.0){c.t = -1.0;return c;}
	float t1 = (-qb + sqrt(qd)) / (2.0 * qa);
	float t2 = (-qb - sqrt(qd)) / (2.0 * qa);
	float t_near = min(t1, t2);
	float t_far = max(t1, t2);
	c.t = t_near;
	if (t_far < 0.0){c.t = -1.0;return c;}
	if (t_near < 0.0){c.t = t_far;c.inside = true;}
	c.p = r.start + c.t * r.dir;
	c.n = normalize(c.p - o.position);
	if (c.inside){c.n *= -1.0;}
	(c.tc).x = 0.5 + atan(-(c.n).z, (c.n).x) / (2.0 * PI);
	(c.tc).y = 0.5 - asin(-(c.n).y) / PI;
	return c;
}

Collision get_closest_collision(Ray r)
{
	float closest = FLT_MAX;
	Collision closest_collision;
	closest_collision.object_index = -1;
	for (int i = 0; i < numObjects; i++)
	{
		Collision c;
		if (objects[i].type == TYPE_ROOM)
		{
			c = intersect_room_box_object(r);
			if (c.t <= 0) continue;
		}
		else if (objects[i].type == TYPE_SPHERE)
		{
			c = intersect_sphere_object(r, objects[i]);
			if (c.t <= 0) continue;
		}
		else if (objects[i].type == TYPE_BOX)
		{
			c = intersect_box_object(r, objects[i]);
			if (c.t <= 0) continue;
		}
		else if (objects[i].type == TYPE_PLANE)
		{
			c = intersect_plane_object(r, objects[i]);
			if (c.t <= 0) continue;
		}
		else if (objects[i].type == TYPE_TRIANGLE)
		{
			c = intersect_triangle_object(r, objects[i]);
			if (c.t <= 0) continue;
		}
		else if (objects[i].type == TYPE_MESH)
		{
			c = intersect_mesh_object(r, objects[i]);
			if (c.t <= 0) continue;
		}
				else if (objects[i].type == TYPE_BVH)
		{
			c = intersect_bvh_object(r, objects[i]);
			if (c.t <= 0) continue;
		}
		else continue;
		if (c.t < closest){closest = c.t;closest_collision = c;closest_collision.object_index = i;}
	}
	return closest_collision;
}

vec3 ads_phong_lighting(Ray r, Collision c)
{
	vec4 ambient = worldAmb_ambient + pointLight_ambient * objects[c.object_index].ambient;
	vec4 diffuse = vec4(0.0);
	vec4 specular = vec4(0.0);
	Ray light_ray;
	light_ray.start = c.p + c.n * 0.01;
	light_ray.dir = normalize(pointLight_position - c.p);
	bool in_shadow = false;
	Collision c_shadow = get_closest_collision(light_ray);
	if ((c_shadow.object_index != -1) && c_shadow.t < length(pointLight_position - c.p)){in_shadow = true;}
	if (in_shadow == false)
	{
		vec3 light_dir = normalize(pointLight_position - c.p);
		vec3 light_ref = normalize(reflect(-light_dir, c.n));
		float cos_theta = dot(light_dir, c.n);
		float cos_phi = dot(normalize(-r.dir), light_ref);
		diffuse = pointLight_diffuse * objects[c.object_index].diffuse * max(cos_theta, 0.0);
		specular = pointLight_specular * objects[c.object_index].specular * pow(max(cos_phi, 0.0), objects[c.object_index].shininess);
	}
	vec4 phong_color = ambient + diffuse + specular;
	return phong_color.rgb;
}

void push(Ray r, int depth, int type)
{
	if (stack_pointer >= stack_size - 1) return;
	Stack_Element element;
	element = null_stack_element;
	element.type = type;
	element.depth = depth;
	element.phase = 1;
	element.ray = r;
	stack_pointer++;
	stack[stack_pointer] = element;
}

Stack_Element pop()
{
	Stack_Element top_stack_element = stack[stack_pointer];
	stack[stack_pointer] = null_stack_element;
	stack_pointer--;
	return top_stack_element;
}

void process_stack_element(int index)
{
	if (popped_stack_element != null_stack_element)
	{
		if (popped_stack_element.type == RAY_TYPE_REFLECTION) stack[index].reflected_color = popped_stack_element.final_color;
		else if (popped_stack_element.type == RAY_TYPE_REFRACTION) stack[index].refracted_color = popped_stack_element.final_color;
		popped_stack_element = null_stack_element;
	}
	Ray r = stack[index].ray;
	Collision c = stack[index].collision;
	switch (stack[index].phase)
	{
		case 1:
			c = get_closest_collision(r);
			if (c.object_index != -1) stack[index].collision = c;
			break;
		case 2:
			stack[index].phong_color = ads_phong_lighting(r, c);
			break;
		case 3:
			if (stack[index].depth < max_depth)
			{
				if (objects[c.object_index].isReflective)
				{
					Ray reflected_ray;
					reflected_ray.start = c.p + c.n * 0.001;
					reflected_ray.dir = reflect(r.dir, c.n);
					push(reflected_ray, stack[index].depth + 1, RAY_TYPE_REFLECTION);
				}
			}
			break;
		case 4:
			if (stack[index].depth < max_depth)
			{
				if (objects[c.object_index].isTransparent)
				{
					Ray refracted_ray;
					refracted_ray.start = c.p - c.n * 0.001;
					float refraction_ratio = 1.0 / objects[c.object_index].IOR;
					if (c.inside) refraction_ratio = 1.0 / refraction_ratio;
					refracted_ray.dir = refract(r.dir, c.n, refraction_ratio);
					push(refracted_ray, stack[index].depth + 1, RAY_TYPE_REFRACTION);
				}
			}
			break;
		case 5:
			if (c.object_index > 0)
			{
				vec3 texColor = vec3(0.0);
				if (objects[c.object_index].hasTexture) texColor = getTextureColor(c.object_index, c.tc);
				vec3 objColor = vec3(0.0);
				if (objects[c.object_index].hasColor) objColor = objects[c.object_index].color;
				vec3 reflected_color = stack[index].reflected_color;
				vec3 refracted_color = stack[index].refracted_color;
				vec3 mixed_color = objColor + texColor;
				if ((objects[c.object_index].isReflective) && (stack[index].depth < max_depth))
					mixed_color = mix(mixed_color, reflected_color, objects[c.object_index].reflectivity);
				if ((objects[c.object_index].isTransparent) && (stack[index].depth < max_depth))
					mixed_color = mix(mixed_color, refracted_color, objects[c.object_index].refractivity);
				stack[index].final_color = 1.5 * mixed_color * stack[index].phong_color;
			}
			if (c.object_index == 0)
			{
				vec3 lightFactor = vec3(1.0);
				if (objects[c.object_index].isReflective) lightFactor = stack[index].phong_color;
				if (objects[c.object_index].hasColor) stack[index].final_color = lightFactor * objects[c.object_index].color;
				else
				{
					if (c.face_index == 0) 
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
				}
			}
			break;

		case 6:
		{
			popped_stack_element = pop();
			return;
		}
	}
	stack[index].phase++;
	return;
}

vec3 raytrace(Ray r)
{
	push(r, 0, RAY_TYPE_REFLECTION);
	while (stack_pointer >= 0)
	{
		int element_index = stack_pointer;
		process_stack_element(element_index);
	}
	return popped_stack_element.final_color;
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
