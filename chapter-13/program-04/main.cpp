#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/soil2.h>
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Torus.h"
#include "Common.h"
using namespace std;
#define NUM_VAOs 1
#define NUM_VBOs 4

float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 1.0f;
float torLocX = 0.0f, torLocY = 0.0f, torLocZ = -1.0f;
GLuint renderingProgram, vao[NUM_VAOs], vbo[NUM_VBOs];
GLuint mvLoc, projLoc, nLoc;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat, rMat;
glm::vec3 currentLightPos, lightLoc(2.0f, 2.0f, 3.0f); 
int width, height;
float aspect, amt = 0.0f;
float lightPos[3];

float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float* matAmb = Utils::goldAmbient(), * matDif = Utils::goldDiffuse(), * matSpe = Utils::goldSpecular();
float matShi = Utils::goldShininess();

Torus myTorus(0.5f, 0.2f, 72);
int numTorusVertices = myTorus.getNumVertices(), numTorusIndices = myTorus.getNumIndices();
float toRadians(float degrees) { return glm::radians(degrees); }
void installLights(glm::mat4 vMatrix) {
    glm::vec3 transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0f)); 
    lightPos[0] = transformed.x;
    lightPos[1] = transformed.y;
    lightPos[2] = transformed.z;
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
void setupVertices() {
    auto indices = myTorus.getIndices();
    auto vertices = myTorus.getVertices();
    auto texCoords = myTorus.getTexCoords();
    auto normals = myTorus.getNormals();
    vector<float> pValues, tValues, nValues;
    for (int i = 0; i < numTorusVertices; ++i) {
        pValues.insert(pValues.end(), { vertices[i].x, vertices[i].y, vertices[i].z });
        tValues.insert(tValues.end(), { texCoords[i].s, texCoords[i].t });
        nValues.insert(nValues.end(), { normals[i].x, normals[i].y, normals[i].z });
    }
    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(NUM_VBOs, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, pValues.size() * sizeof(float), pValues.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, tValues.size() * sizeof(float), tValues.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, nValues.size() * sizeof(float), nValues.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
}
void init(GLFWwindow* window) {
    renderingProgram = Utils::createShaderProgram("vertShader.glsl", "geomShader.glsl", "fragShader.glsl");
    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / height;
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
    amt = currentTime * 25.0f;
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
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glDrawElements(GL_TRIANGLES, numTorusIndices, GL_UNSIGNED_INT, 0);
}
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
    aspect = (float)newWidth / newHeight;
    glViewport(0, 0, newWidth, newHeight);
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}
int main() {
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 800, "program 1304", NULL, NULL);
    if (!window) exit(EXIT_FAILURE);
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
