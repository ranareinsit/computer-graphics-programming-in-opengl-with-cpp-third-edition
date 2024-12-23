#include "Common.h"
#include "ImportedModel.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
using namespace std;

#define RAYTRACE_RENDER_WIDTH 512
#define RAYTRACE_RENDER_HEIGHT 512
#define numVAOs 1
#define numVBOs 2

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
GLuint ssbo[3];

struct ModelData
{
	vector<float> pvalues;
	vector<float> tvalues;
	vector<float> nvalues;
};

ModelData mdata;
bool loadModel(string filePath) {
	try {
		// Clear previous model data
		mdata.pvalues.clear();
		mdata.tvalues.clear();
		mdata.nvalues.clear();

		ImportedModel model(filePath.c_str());
		// ImportedModel dolphinObj("dolphinLowPoly.obj");
		vector<glm::vec3> vert = model.getVertices();
		vector<glm::vec2> tex = model.getTextureCoords();
		vector<glm::vec3> norm = model.getNormals();
		int numObjVertices = model.getNumVertices();

		// Pre-allocate memory
		mdata.pvalues.reserve(numObjVertices * 3);
		mdata.tvalues.reserve(numObjVertices * 2);
		mdata.nvalues.reserve(numObjVertices * 3);

		for (size_t i = 0; i < numObjVertices; i++) {
			// Add vertices
			mdata.pvalues.push_back(vert[i].x);
			mdata.pvalues.push_back(vert[i].y);
			mdata.pvalues.push_back(vert[i].z);
			
			// Add default texture coordinates if none exist
			if (tex.empty()) {
				mdata.tvalues.push_back(0.0f);
				mdata.tvalues.push_back(0.0f);
			} else {
				mdata.tvalues.push_back(tex[i].x);
				mdata.tvalues.push_back(tex[i].y);
			}
			
			// Add normals
			mdata.nvalues.push_back(norm[i].x);
			mdata.nvalues.push_back(norm[i].y);
			mdata.nvalues.push_back(norm[i].z);
		}

		cout << "Loaded " << numObjVertices / 3 << " triangles from " << filePath << endl;
		return true;
	}
	catch (const std::exception& e) {
		cerr << "Error loading model " << filePath << ": " << e.what() << endl;
		return false;
	}
}

void init(GLFWwindow* window)
{
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


	glGenBuffers(3, ssbo);
	loadModel("icosahedron.obj");

	float* pv = mdata.pvalues.data();
	float* tv = mdata.tvalues.data();
	float* nv = mdata.nvalues.data();
	int pvSize = int(mdata.pvalues.size() * 4);
	int tvSize = int(mdata.tvalues.size() * 4);
	int nvSize = int(mdata.nvalues.size() * 4);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, pvSize, pv, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, tvSize, tv, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nvSize, nv, GL_STATIC_DRAW);
}
void display(GLFWwindow* window, double currentTime)
{
	glUseProgram(raytraceComputeShader);
	glBindImageTexture(0, screenTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, marbleTexture);
	glActiveTexture(GL_TEXTURE0);
	//
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[2]);
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

void setWindowSizeCallback(GLFWwindow* win, int newWidth, int newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
}

int main(void)
{
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
		if (safe < 3)
		{
			std::cout << "hmm..." << std::endl;
			display(window, glfwGetTime());
			glfwSwapBuffers(window);
			glfwPollEvents();
			safe++;
		}
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
