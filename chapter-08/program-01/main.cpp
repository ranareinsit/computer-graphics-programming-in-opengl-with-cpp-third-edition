#include "Common.h"
#include "ImportedModel.h"
#include "Torus.h"
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.inl>
#include <glm/ext/matrix_transform.inl>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.inl>
#include <vector>
using namespace std;
void passOne(float currentTime);
void passTwo(float currentTime);
static float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }
constexpr auto numVAOs = 1;
constexpr auto numVBOs = 5;
GLuint renderingProgram1, renderingProgram2;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
ImportedModel pyramid("pyr.obj");
Torus myTorus(0.1f, 0.1f, 36);
int numPyramidVertices, numTorusVertices, numTorusIndices;
glm::vec3 torusLoc(0.0f, 1.0f, 0.5f);
glm::vec3 pyrLoc(0.0f, 0.0f, 0.0f);
glm::vec3 cameraLoc(0.0f, 0.0f, 3.0f);
glm::vec3 lightLoc(0.0f, 2.0f, 0.5f);
float globalAmbient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float* gMatAmb = Utils::goldAmbient();
float* gMatDif = Utils::goldDiffuse();
float* gMatSpe = Utils::goldSpecular();
float gMatShi = Utils::goldShininess();
float* bMatAmb = Utils::bronzeAmbient();
float* bMatDif = Utils::bronzeDiffuse();
float* bMatSpe = Utils::bronzeSpecular();
float bMatShi = Utils::bronzeShininess();
float thisAmb[4], thisDif[4], thisSpe[4], matAmb[4], matDif[4], matSpe[4];
float thisShi, matShi;
int scSizeX, scSizeY;
GLuint shadowTex, shadowBuffer;
glm::mat4 lightVmatrix;
glm::mat4 lightPmatrix;
glm::mat4 shadowMVP1;
glm::mat4 shadowMVP2;
glm::mat4 b;
GLuint mvLoc, projLoc, nLoc, sLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
glm::vec3 currentLightPos, transformed;
float lightPos[3];
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
glm::vec3 origin(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

static void installLights(int renderingProgram, glm::mat4 vMatrix) {
	transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;
	matSpe[3] = thisSpe[3];
	matSpe[2] = thisSpe[2];
	matSpe[1] = thisSpe[1];
	matSpe[0] = thisSpe[0];
	matDif[3] = thisDif[3];
	matDif[2] = thisDif[2];
	matDif[1] = thisDif[1];
	matDif[0] = thisDif[0];
	matAmb[3] = thisAmb[3];
	matAmb[2] = thisAmb[2];
	matAmb[1] = thisAmb[1];
	matAmb[0] = thisAmb[0];
	matShi = thisShi;
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "light.position");
	mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mdiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mspecLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");
	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
	glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
	glProgramUniform4fv(renderingProgram, mambLoc, 1, matAmb);
	glProgramUniform4fv(renderingProgram, mdiffLoc, 1, matDif);
	glProgramUniform4fv(renderingProgram, mspecLoc, 1, matSpe);
	glProgramUniform1f(renderingProgram, mshiLoc, matShi);
}
static void setupVertices(void) {
	numPyramidVertices = pyramid.getNumVertices();
	vector<glm::vec3> vert = pyramid.getVertices();
	vector<glm::vec3> norm = pyramid.getNormals();
	vector<float> pyramidPvalues;
	vector<float> pyramidNvalues;
	for (int i = 0; i < numPyramidVertices; i++) {
		pyramidPvalues.push_back((vert[i]).x);
		pyramidPvalues.push_back((vert[i]).y);
		pyramidPvalues.push_back((vert[i]).z);
		pyramidNvalues.push_back((norm[i]).x);
		pyramidNvalues.push_back((norm[i]).y);
		pyramidNvalues.push_back((norm[i]).z);
	}
	numTorusVertices = myTorus.getNumVertices();
	numTorusIndices = myTorus.getNumIndices();
	vector<int> ind = myTorus.getIndices();
	vert = myTorus.getVertices();
	norm = myTorus.getNormals();
	vector<float> torusPvalues;
	vector<float> torusNvalues;
	for (int i = 0; i < numTorusVertices; i++) {
		torusPvalues.push_back(vert[i].x);
		torusPvalues.push_back(vert[i].y);
		torusPvalues.push_back(vert[i].z);
		torusNvalues.push_back(norm[i].x);
		torusNvalues.push_back(norm[i].y);
		torusNvalues.push_back(norm[i].z);
	}
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, torusPvalues.size() * 4, &torusPvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, pyramidPvalues.size() * 4, &pyramidPvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, torusNvalues.size() * 4, &torusNvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, pyramidNvalues.size() * 4, &pyramidNvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * 4, &ind[0], GL_STATIC_DRAW);
}
static void setupShadowBuffers(GLFWwindow* window) {
	glfwGetFramebufferSize(window, &width, &height);
	scSizeX = width;
	scSizeY = height;
	glGenFramebuffers(1, &shadowBuffer);
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, scSizeX, scSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
static void init(GLFWwindow* window) {
	renderingProgram1 = Utils::createShaderProgram("./vert1Shader.glsl", "./frag1Shader.glsl");
	renderingProgram2 = Utils::createShaderProgram("./vert2Shader.glsl", "./frag2Shader.glsl");
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0f, aspect, 0.1f, 1000.0f);
	setupVertices();
	setupShadowBuffers(window);
	b = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f
	);
}
static glm::mat4 eulerRotation(glm::mat4 current, float theta_x, float theta_y, float theta_z) {
	glm::mat4 rotationX = glm::rotate(current, theta_x, glm::vec3(0.5f, 0.0f, 0.0f));
	glm::mat4 rotationY = glm::rotate(current, theta_y, glm::vec3(0.0f, 0.5f, 0.0f));
	glm::mat4 rotationZ = glm::rotate(current, theta_z, glm::vec3(0.0f, 0.0f, 0.5f));
	return rotationZ;
}
static void display(GLFWwindow* window, double currentTime) {
	float fct = float(currentTime) / 2;
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	currentLightPos = glm::vec3(lightLoc);
	lightVmatrix = glm::lookAt(currentLightPos, origin, up);
	lightPmatrix = glm::perspective(toRadians(60.0f), aspect, 0.1f, 1000.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);
	passOne(fct);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glDrawBuffer(GL_FRONT);
	passTwo(fct);
}

void passOne(float currentTime) {
	glUseProgram(renderingProgram1);
	mMat = glm::translate(glm::mat4(1.0f), torusLoc);
	mMat = glm::rotate(mMat, toRadians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mMat = eulerRotation(mMat, currentTime, currentTime, currentTime);
	shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glDrawElements(GL_TRIANGLES, numTorusIndices, GL_UNSIGNED_INT, 0);

	mMat = glm::translate(glm::mat4(1.0f), pyrLoc);
	mMat = glm::rotate(mMat, toRadians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mMat = glm::rotate(mMat, toRadians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, numPyramidVertices);
}
void passTwo(float currentTime) {
	glUseProgram(renderingProgram2);
	mvLoc = glGetUniformLocation(renderingProgram2, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram2, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram2, "norm_matrix");
	sLoc = glGetUniformLocation(renderingProgram2, "shadowMVP");

	thisAmb[0] = bMatAmb[0]; thisAmb[1] = bMatAmb[1]; thisAmb[2] = bMatAmb[2];
	thisDif[0] = bMatDif[0]; thisDif[1] = bMatDif[1]; thisDif[2] = bMatDif[2];
	thisSpe[0] = bMatSpe[0]; thisSpe[1] = bMatSpe[1]; thisSpe[2] = bMatSpe[2];
	thisShi = bMatShi;
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraLoc.x, -cameraLoc.y, -cameraLoc.z));
	mMat = glm::translate(glm::mat4(1.0f), torusLoc);
	mMat = glm::rotate(mMat, toRadians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mMat = eulerRotation(mMat, currentTime, currentTime, currentTime);
	currentLightPos = glm::vec3(lightLoc);
	installLights(renderingProgram2, vMat);
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glDrawElements(GL_TRIANGLES, numTorusIndices, GL_UNSIGNED_INT, 0);

	thisAmb[0] = gMatAmb[0]; thisAmb[1] = gMatAmb[1]; thisAmb[2] = gMatAmb[2];
	thisDif[0] = gMatDif[0]; thisDif[1] = gMatDif[1]; thisDif[2] = gMatDif[2];
	thisSpe[0] = gMatSpe[0]; thisSpe[1] = gMatSpe[1]; thisSpe[2] = gMatSpe[2];
	thisShi = gMatShi;
	mMat = glm::translate(glm::mat4(1.0f), pyrLoc);
	mMat = glm::rotate(mMat, toRadians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mMat = glm::rotate(mMat, toRadians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	currentLightPos = glm::vec3(lightLoc);
	installLights(renderingProgram2, vMat);
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, numPyramidVertices);
}
static void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	pMat = glm::perspective(1.0f, aspect, 0.1f, 1000.0f);
}
int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 800, "program0801", NULL, NULL);
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
