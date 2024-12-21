#include "Common.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/soil2.h>
#include <iostream>
#include <string>

float toRadians(float degrees) { return degrees * 2.0f * 3.14159f / 360.0f; }

#define NUM_VAOS 1

float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 0.7f;
float terLocX = 0.0f, terLocY = 0.0f, terLocZ = 0.0f;
float transDir = 1.0f;
double prevTime = 0.0;

GLuint renderingProgram, vao[NUM_VAOS], mvpLoc;
GLuint squareMoonTexture, squareMoonHeight;

int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvpMat;

void init(GLFWwindow* window) {
    renderingProgram = Utils::createShaderProgram("vertShader.glsl", "tessCShader.glsl", "tessEShader.glsl", "fragShader.glsl");

    glfwGetFramebufferSize(window, &width, &height);
    aspect = static_cast<float>(width) / height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

    squareMoonTexture = Utils::loadTexture("squareMoonMap.jpg");
    squareMoonHeight = Utils::loadTexture("squareMoonBump.jpg");

    glGenVertexArrays(NUM_VAOS, vao);
    glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);

    vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
    cameraZ += transDir * (currentTime - prevTime) * 0.1f;
    transDir = (cameraZ > 0.9f) ? -1.0f : (cameraZ < 0.3f) ? 1.0f : transDir;
    prevTime = currentTime;

    mMat = glm::translate(glm::mat4(1.0f), glm::vec3(terLocX, terLocY, terLocZ));
    mMat = glm::rotate(mMat, toRadians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mvpMat = pMat * vMat * mMat;

    mvpLoc = glGetUniformLocation(renderingProgram, "mvp_matrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvpMat));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, squareMoonTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, squareMoonHeight);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
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
    GLFWwindow* window = glfwCreateWindow(800, 800, "program 1204", nullptr, nullptr);
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
