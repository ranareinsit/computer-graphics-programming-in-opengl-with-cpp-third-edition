#include "Common.h"
#include "Torus.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/soil2.h>
using namespace std;
const float TO_RADIANS = 3.14159f / 180.0f;
const int numVAOs = 1, numVBOs = 4;
GLuint renderingProgram, vao[numVAOs], vbo[numVBOs];
Torus myTorus(0.5f, 0.2f, 48);
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat, rMat;
glm::vec3 lightLoc(5.0f, 2.0f, 2.0f), currentLightPos;
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 1.0f;
float torLocX = 0.0f, torLocY = 0.0f, torLocZ = -1.0f;
int width, height;
float aspect;
GLuint mvLoc, projLoc, nLoc, globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
float lightPos[3];
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float* matAmb = Utils::goldAmbient(), * matDif = Utils::goldDiffuse(), * matSpe = Utils::goldSpecular();
float matShi = Utils::goldShininess();
float toRadians(float degrees) { return degrees * TO_RADIANS; }
void setupVertices() {
	vector<int> ind = myTorus.getIndices();
	vector<glm::vec3> vert = myTorus.getVertices();
	vector<glm::vec2> tex = myTorus.getTexCoords();
	vector<glm::vec3> norm = myTorus.getNormals();
	vector<float> pvalues, tvalues, nvalues;
	for (int i = 0; i < myTorus.getNumVertices(); i++) {
		pvalues.push_back(vert[i].x); pvalues.push_back(vert[i].y); pvalues.push_back(vert[i].z);
		tvalues.push_back(tex[i].s); tvalues.push_back(tex[i].t);
		nvalues.push_back(norm[i].x); nvalues.push_back(norm[i].y); nvalues.push_back(norm[i].z);
	}
	glGenVertexArrays(1, vao); glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); glBufferData(GL_ARRAY_BUFFER, pvalues.size() * sizeof(float), &pvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); glBufferData(GL_ARRAY_BUFFER, tvalues.size() * sizeof(float), &tvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); glBufferData(GL_ARRAY_BUFFER, nvalues.size() * sizeof(float), &nvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]); glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(int), &ind[0], GL_STATIC_DRAW);
}
void installLights(glm::mat4 vMatrix) {
	glm::vec3 transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x; lightPos[1] = transformed.y; lightPos[2] = transformed.z;
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
void init(GLFWwindow* window) {
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "geomShader.glsl", "fragShader.glsl");
	glfwGetFramebufferSize(window, &width, &height);
	aspect = static_cast<float>(width) / height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
	setupVertices();
}
void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(torLocX, torLocY, torLocZ));
	mMat = glm::rotate(mMat, toRadians(35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	float amt = static_cast<float>(currentTime) * 25.0f;
	rMat = glm::rotate(glm::mat4(1.0f), toRadians(amt), glm::vec3(0.0f, 0.0f, 1.0f));
	currentLightPos = glm::vec3(rMat * glm::vec4(lightLoc, 1.0f));
	installLights(vMat);
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
	glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);
}
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
	aspect = static_cast<float>(newWidth) / newHeight;
	glViewport(0, 0, newWidth, newHeight);
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}
int main() {
	if (!glfwInit()) exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 800, "program 130101", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) exit(EXIT_FAILURE);
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
