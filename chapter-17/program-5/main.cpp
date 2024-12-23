#include "Common.h"
#include "ImportedModel.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <algorithm>
using namespace std;

// Constants
#define RAYTRACE_RENDER_WIDTH 512
#define RAYTRACE_RENDER_HEIGHT 512
#define numVAOs 1
#define numVBOs 2

// Global Variables
int windowWidth = 512;
int windowHeight = 512;
int workGroupsX = RAYTRACE_RENDER_WIDTH;
int workGroupsY = RAYTRACE_RENDER_HEIGHT;
int workGroupsZ = 1;
unsigned char* screenTexture;

GLuint screenTextureID;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
GLuint screenQuadShader, raytraceComputeShader;
GLuint marbleTexture;
GLuint ssbo[16];
std::vector<glm::vec4> centroids;
vector<float> serialBVHs{};

// Forward Declarations
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void setWindowSizeCallback(GLFWwindow* win, int newWidth, int newHeight);

// Structs
struct ModelData {
	vector<float> pvalues;
	vector<float> tvalues;
	vector<float> nvalues;
	vector<float> bvhValues;
};

struct BoundingVolume {
	float minX, maxX, minY, maxY, minZ, maxZ;
};

struct BVHNode {
	int triangleIdx = -1;
	BoundingVolume bounds;
	BVHNode* left;
	BVHNode* right;
};

ModelData mdata;

// Bounding Volume Functions
void initBoundingVolume(BoundingVolume& bv) {
	bv.minX = 0.f; bv.maxX = 0.f;
	bv.minY = 0.f; bv.maxY = 0.f;
	bv.minZ = 0.f; bv.maxZ = 0.f;
}

void setBoundingVolume(BoundingVolume& bv, float mnX, float mxX, float mnY, float mxY, float mnZ, float mxZ) {
	bv.minX = mnX; bv.maxX = mxX;
	bv.minY = mnY; bv.maxY = mxY;
	bv.minZ = mnZ; bv.maxZ = mxZ;
}

void setBoundingVolumeFromBV(BoundingVolume& bv, BoundingVolume bvSource) {
	bv.minX = bvSource.minX; bv.maxX = bvSource.maxX;
	bv.minY = bvSource.minY; bv.maxY = bvSource.maxY;
	bv.minZ = bvSource.minZ; bv.maxZ = bvSource.maxZ;
}

void setBoundingVolumeFromTriangle(BoundingVolume& bv, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
	bv.minX = std::min(p0.x, std::min(p1.x, p2.x));
	bv.maxX = std::max(p0.x, std::max(p1.x, p2.x));
	bv.minY = std::min(p0.y, std::min(p1.y, p2.y));
	bv.maxY = std::max(p0.y, std::max(p1.y, p2.y));
	bv.minZ = std::min(p0.z, std::min(p1.z, p2.z));
	bv.maxZ = std::max(p0.z, std::max(p1.z, p2.z));
}

void setBoundingVolumeFromUnion(BoundingVolume& bv, BoundingVolume bv0, BoundingVolume bv1) {
	bv.minX = std::min(bv0.minX, bv1.minX);
	bv.maxX = std::max(bv0.maxX, bv1.maxX);
	bv.minY = std::min(bv0.minY, bv1.minY);
	bv.maxY = std::max(bv0.maxY, bv1.maxY);
	bv.minZ = std::min(bv0.minZ, bv1.minZ);
	bv.maxZ = std::max(bv0.maxZ, bv1.maxZ);
}

// BVH Functions
void initBVHNode(BVHNode& n) {
	n.triangleIdx = -1;
	initBoundingVolume(n.bounds);
}

void getCentroidsWithIndices(glm::vec3 vertices[], int numVertices) {
	centroids.clear();
	glm::vec4 centroidWithIndex;
	int triCount = numVertices / 3;

	for (int i = 0; i < triCount; i++) {
		glm::vec3 p0 = glm::vec3(vertices[i * 3 + 0]);
		glm::vec3 p1 = glm::vec3(vertices[i * 3 + 1]);
		glm::vec3 p2 = glm::vec3(vertices[i * 3 + 2]);
		centroidWithIndex.x = (p0.x + p1.x + p2.x) / 3.f;
		centroidWithIndex.y = (p0.y + p1.y + p2.y) / 3.f;
		centroidWithIndex.z = (p0.z + p1.z + p2.z) / 3.f;
		centroidWithIndex.w = (float)i;
		centroids.push_back(centroidWithIndex);
	}
}

void sortCentroids(int axis, int left, int right) {
	std::sort(
		centroids.begin() + left,
		centroids.begin() + right + 1,
		[axis](const glm::vec4& a, const glm::vec4& b) {
			if (axis == 0) return a.x < b.x;
			else if (axis == 1) return a.y < b.y;
			else return a.z < b.z;
		});
}

void serializeBVHRecursively(BVHNode& node) {
	serialBVHs.push_back((float)node.triangleIdx);
	serialBVHs.push_back(node.bounds.minX);
	serialBVHs.push_back(node.bounds.maxX);
	serialBVHs.push_back(node.bounds.minY);
	serialBVHs.push_back(node.bounds.maxY);
	serialBVHs.push_back(node.bounds.minZ);
	serialBVHs.push_back(node.bounds.maxZ);

	int indexOfLeftIndex = (int)serialBVHs.size();
	serialBVHs.push_back(-1.f);
	int indexOfRightIndex = (int)serialBVHs.size();
	serialBVHs.push_back(-1.f);

	if (node.left != NULL) {
		serialBVHs[indexOfLeftIndex] = (float)serialBVHs.size();
		serializeBVHRecursively(*node.left);
	}

	if (node.right != NULL) {
		serialBVHs[indexOfRightIndex] = (float)serialBVHs.size();
		serializeBVHRecursively(*node.right);
	}
}

BVHNode* constructBVHRecursively(glm::vec3 vertices[], int numVertices, int axis, int left, int right) {
	int count = right - left + 1;
	if (count == 1) {
		BVHNode* node = new BVHNode();
		int i = (int)centroids[left].w;
		node->triangleIdx = i;
		node->left = NULL;
		node->right = NULL;
		BoundingVolume nodeBounds = { 0,0,0,0,0,0 };
		setBoundingVolumeFromTriangle(nodeBounds, vertices[i * 3 + 0], vertices[i * 3 + 1],
			vertices[i * 3 + 2]);
		node->bounds = nodeBounds;
		return node;
	}
	else {
		sortCentroids(axis, left, right);
		int nextAxis = (axis + 1) % 3;
		int middle = left + count / 2;

		BVHNode* node = new BVHNode();
		node->left = constructBVHRecursively(vertices, numVertices, nextAxis, left, middle - 1);
		node->right = constructBVHRecursively(vertices, numVertices, nextAxis, middle, right);

		BoundingVolume bVleft = { 0, 0, 0, 0, 0, 0 };
		BoundingVolume bVright = { 0, 0, 0, 0, 0, 0 };
		setBoundingVolumeFromBV(bVleft, (node->left)->bounds);
		setBoundingVolumeFromBV(bVright, (node->right)->bounds);
		setBoundingVolumeFromUnion(node->bounds, bVleft, bVright);

		node->triangleIdx = -1;
		return node;
	}
}

BVHNode constructBVH(glm::vec3 vertices[], int numVertices) {
	getCentroidsWithIndices(vertices, numVertices);
	BVHNode* root = constructBVHRecursively(vertices, numVertices, 0, 0, (int)centroids.size() - 1);
	BVHNode node = *root;
	delete root;
	return node;
}

// Model Loading Functions
ModelData loadModel(const char* filePath) {
	ImportedModel model(filePath);
	vector<glm::vec3> vert = model.getVertices();
	vector<glm::vec2> tex = model.getTextureCoords();
	vector<glm::vec3> norm = model.getNormals();
	int numObjVertices = model.getNumVertices();
	cout << "triangles: " << numObjVertices / 3 << endl;

	mdata.pvalues.reserve(numObjVertices * 3);
	mdata.tvalues.reserve(numObjVertices * 2);
	mdata.nvalues.reserve(numObjVertices * 3);

	for (size_t i = 0; i < numObjVertices; i++) {
		mdata.pvalues.push_back(vert[i].x);
		mdata.pvalues.push_back(vert[i].y);
		mdata.pvalues.push_back(vert[i].z);

		if (tex.empty()) {
			mdata.tvalues.push_back(0.0f);
			mdata.tvalues.push_back(0.0f);
		}
		else {
			mdata.tvalues.push_back(tex[i].x);
			mdata.tvalues.push_back(tex[i].y);
		}

		mdata.nvalues.push_back(norm[i].x);
		mdata.nvalues.push_back(norm[i].y);
		mdata.nvalues.push_back(norm[i].z);
	}

	BVHNode bvhRoot = constructBVH(vert.data(), numObjVertices);
	serializeBVHRecursively(bvhRoot);
	for (size_t i = 0; i < serialBVHs.size(); i++) {
		mdata.bvhValues.push_back(serialBVHs[i]);
	}

	return mdata;
}

void loadModels(char* filePath[], int numFiles) {
	for (size_t i = 0; i < numFiles; i++) {
		serialBVHs.clear();
		cout << "first TriIdx: " << filePath[i] << " = " << (mdata.pvalues.size() / 9) << endl;
		cout << "bvhRootIdx: " << filePath[i] << " = " << (mdata.bvhValues.size()) << endl;
		loadModel(filePath[i]);
	}
}

// OpenGL Functions
void init(GLFWwindow* window) {
	Utils::displayComputeShaderLimits();
	screenTexture = (unsigned char*)malloc(sizeof(unsigned char) * 4 * RAYTRACE_RENDER_WIDTH * RAYTRACE_RENDER_HEIGHT);
	memset(screenTexture, 0, sizeof(char) * 4 * RAYTRACE_RENDER_WIDTH * RAYTRACE_RENDER_HEIGHT);
	for (int i = 0; i < RAYTRACE_RENDER_HEIGHT; i++)
	{
		for (int j = 0; j < RAYTRACE_RENDER_WIDTH; j++)
		{
			screenTexture[i * RAYTRACE_RENDER_WIDTH * 4 + j * 4 + 0] = 250;
			screenTexture[i * RAYTRACE_RENDER_WIDTH * 4 + j * 4 + 1] = 128;
			screenTexture[i * RAYTRACE_RENDER_WIDTH * 4 + j * 4 + 2] = 255;
			screenTexture[i * RAYTRACE_RENDER_WIDTH * 4 + j * 4 + 3] = 255;
		}
	}
	glGenTextures(1, &screenTextureID);
	glBindTexture(GL_TEXTURE_2D, screenTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RAYTRACE_RENDER_WIDTH, RAYTRACE_RENDER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)screenTexture);
	const float windowQuadVerts[] = {
		-1.0f, 1.0f, 0.3f, -1.0f, -1.0f, 0.3f, 1.0f, -1.0f, 0.3f,
		1.0f, -1.0f, 0.3f, 1.0f, 1.0f, 0.3f, -1.0f, 1.0f, 0.3f };
	const float windowQuadUVs[] = {
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windowQuadVerts), windowQuadVerts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windowQuadUVs), windowQuadUVs, GL_STATIC_DRAW);
	raytraceComputeShader = Utils::createShaderProgram("raytraceComputeShader.glsl");
	screenQuadShader = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	marbleTexture = Utils::loadTexture("ice.jpg");

	const char* OBJfilenames[] = {
		"dolphinLowPoly.obj",
		"pyr.obj",
		"icosahedron.obj",
		"icosahedron.obj",
		"pyr.obj"
	};

	loadModels((char**)OBJfilenames, 5);
	glGenBuffers(4, ssbo);

	float* pv = mdata.pvalues.data();
	float* tv = mdata.tvalues.data();
	float* nv = mdata.nvalues.data();
	int pvSize = int(mdata.pvalues.size() * 4);
	int tvSize = int(mdata.tvalues.size() * 4);
	int nvSize = int(mdata.nvalues.size() * 4);
	float* bv = mdata.bvhValues.data();
	int bvhSize = int(mdata.bvhValues.size()) * 4;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, pvSize, pv, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, tvSize, tv, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nvSize, nv, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[3]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bvhSize, bv, GL_STATIC_DRAW);

}

void display(GLFWwindow* window, double currentTime) {
	glUseProgram(raytraceComputeShader);
	glBindImageTexture(0, screenTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, marbleTexture);
	glActiveTexture(GL_TEXTURE0);
	//
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[2]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo[3]);
	//
	glDispatchCompute(workGroupsX, workGroupsY, workGroupsZ);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(screenQuadShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTextureID);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void setWindowSizeCallback(GLFWwindow* win, int newWidth, int newHeight) {
	glViewport(0, 0, newWidth, newHeight);
}

int main(void) {
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "program 1703", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK)
	{
		exit(EXIT_FAILURE);
	}
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, setWindowSizeCallback);
	init(window);
	int safe = 0;
	while (!glfwWindowShouldClose(window))
	{
		if (safe < 1)
		{
			std::cout << "hmm..." << std::endl;
			display(window, glfwGetTime());
			glfwSwapBuffers(window);
			safe++;
		}
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
