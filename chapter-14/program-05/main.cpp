#include "Common.h"
#include "ImportedModel.h"
#include <cmath>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 
#include <iostream>
#include <random>
#include <SOIL2/soil2.h>
#include <string>
using namespace std;
float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }
#define numVAOs 1
#define numVBOs 3
float cameraX, cameraY, cameraZ;
float objLocX, objLocY, objLocZ;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
glm::vec3 lightLoc = glm::vec3(-2.0f, 3.0f, 0.6f);
GLuint noiseTexture;
const int noisePrecision = 64;
const int noiseWidth = noisePrecision;
const int noiseHeight = noisePrecision;
const int noiseDepth = noisePrecision;
double noise[noiseWidth][noiseHeight][noiseDepth];
GLuint mvLoc, projLoc, nLoc;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
glm::vec3 currentLightPos;
float lightPos[3];
ImportedModel dolphinObj("dolphinLowPoly.obj");
int numDolphinVertices;
float globalAmbient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float matShi = 75.0f;
double smoothNoise(double zoom, double x1, double y1, double z1) {
	double fractX = x1 - (int)x1;
	double fractY = y1 - (int)y1;
	double fractZ = z1 - (int)z1;
	double x2 = x1 - 1; if (x2 < 0) x2 = round(noiseWidth / zoom) - 1.0;
	double y2 = y1 - 1; if (y2 < 0) y2 = round(noiseHeight / zoom) - 1.0;
	double z2 = z1 - 1; if (z2 < 0) z2 = round(noiseDepth / zoom) - 1.0;
	double value = 0.0;
	value += fractX * fractY * fractZ * noise[(int)x1][(int)y1][(int)z1];
	value += (1.0 - fractX) * fractY * fractZ * noise[(int)x2][(int)y1][(int)z1];
	value += fractX * (1.0 - fractY) * fractZ * noise[(int)x1][(int)y2][(int)z1];
	value += (1.0 - fractX) * (1.0 - fractY) * fractZ * noise[(int)x2][(int)y2][(int)z1];
	value += fractX * fractY * (1.0 - fractZ) * noise[(int)x1][(int)y1][(int)z2];
	value += (1.0 - fractX) * fractY * (1.0 - fractZ) * noise[(int)x2][(int)y1][(int)z2];
	value += fractX * (1.0 - fractY) * (1.0 - fractZ) * noise[(int)x1][(int)y2][(int)z2];
	value += (1.0 - fractX) * (1.0 - fractY) * (1.0 - fractZ) * noise[(int)x2][(int)y2][(int)z2];
	return value;
}
double turbulence(double x, double y, double z, double maxZoom) {
	double sum = 0.0, zoom = maxZoom;
	while (zoom >= 0.9) {
		sum = sum + smoothNoise(zoom, x / zoom, y / zoom, z / zoom) * zoom;
		zoom = zoom / 2.0;
	}
	sum = 128.0 * sum / maxZoom;
	return sum;
}
double logistic(double x) {
	double k = 3.0;
	return (1.0 / (1.0 + pow(2.718, -k * x)));
}
void fillDataArray(GLubyte data[]) {
	double veinFrequency = 2.0;
	double turbPower = 4.0;
	double maxZoom = 32.0;
	for (int i = 0; i < noiseWidth; i++) {
		for (int j = 0; j < noiseHeight; j++) {
			for (int k = 0; k < noiseDepth; k++) {
				double xyzValue = 
					(float)i / 
					noiseWidth + 
					(float)j / 
					noiseHeight + 
					(float)k / 
					noiseDepth + 
					turbPower * 
					turbulence(i, j, k, maxZoom) / 
					256.0;
				double sineValue = logistic(abs(sin(xyzValue * 3.14159 * veinFrequency)));
				sineValue = max(-1.0, min(sineValue * 1.25 - 0.20, 1.0));
				float redPortion = 255.0f * (float)sineValue;
				float greenPortion = 255.0f * (float)min(sineValue * 1.5 - 0.25, 1.0);
				float bluePortion = 255.0f * (float)sineValue;
				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 0] = (GLubyte)redPortion;
				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 1] = (GLubyte)greenPortion;
				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 2] = (GLubyte)bluePortion;
				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 3] = (GLubyte)255;
			}
		}
	}
}
GLuint buildNoiseTexture() {
	GLuint textureID;
	GLubyte* data = new GLubyte[noiseWidth * noiseHeight * noiseDepth * 4];
	fillDataArray(data);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_3D, textureID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, noiseWidth, noiseHeight, noiseDepth);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, noiseWidth, noiseHeight, noiseDepth, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	delete[] data;
	return textureID;
}
void generateNoise() {
	for (int x = 0; x < noiseWidth; x++) {
		for (int y = 0; y < noiseHeight; y++) {
			for (int z = 0; z < noiseDepth; z++) {
				noise[x][y][z] = (double)rand() / (RAND_MAX + 1.0);
			}
		}
	}
}
void installLights(glm::mat4 vMatrix) {
	glm::vec3 transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "light.position");
	mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");
	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
	glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
	glProgramUniform1f(renderingProgram, mshiLoc, matShi);
}
void setupVertices(void) {
	numDolphinVertices = dolphinObj.getNumVertices();
	std::vector<glm::vec3> vert = dolphinObj.getVertices();
	std::vector<glm::vec2> tex = dolphinObj.getTextureCoords();
	std::vector<glm::vec3> norm = dolphinObj.getNormals();
	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;
	for (int i = 0; i < numDolphinVertices; i++) {
		pvalues.push_back((vert[i]).x);
		pvalues.push_back((vert[i]).y);
		pvalues.push_back((vert[i]).z);
		tvalues.push_back((tex[i]).s);
		tvalues.push_back((tex[i]).t);
		nvalues.push_back((norm[i]).x);
		nvalues.push_back((norm[i]).y);
		nvalues.push_back((norm[i]).z);
	}
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);
}
void init(GLFWwindow* window) {
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	cameraX = 0.0f; cameraY = 0.0f; cameraZ = 1.7f;
	objLocX = -0.15f; objLocY = 0.0f; objLocZ = 0.0f;
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
	setupVertices();
	generateNoise();
	noiseTexture = buildNoiseTexture();
}
void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(objLocX, objLocY, objLocZ));
	mMat = glm::rotate(mMat, toRadians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	currentLightPos = glm::vec3(lightLoc.x, lightLoc.y, lightLoc.z);
	installLights(vMat);
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, noiseTexture);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, numDolphinVertices);
}
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}
int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 800, "program 1405", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, window_size_callback);
	init(window);
	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
