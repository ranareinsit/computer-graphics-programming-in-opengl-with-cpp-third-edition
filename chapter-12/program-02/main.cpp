#include "Common.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/soil2.h>
float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 4.0f;
float terLocX = 0.0f, terLocY = 0.0f, terLocZ = 0.0f;
GLuint renderingProgram, vao, floorTexture;
GLuint mvpLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvpMat;
void init(GLFWwindow* window) {
    renderingProgram = Utils::createShaderProgram("vertShader.glsl", "tessCShader.glsl", "tessEShader.glsl", "fragShader.glsl");
    glfwGetFramebufferSize(window, &width, &height);
    aspect = static_cast<float>(width) / height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
    floorTexture = Utils::loadTexture("floor_color.jpg");
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}
void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);
    vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
    mMat = glm::translate(glm::mat4(1.0f), glm::vec3(terLocX, terLocY, terLocZ));
    mMat = glm::rotate(mMat, toRadians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mMat = glm::rotate(mMat, float(currentTime * 0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
    mvpMat = pMat * vMat * mMat;
    mvpLoc = glGetUniformLocation(renderingProgram, "mvp_matrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvpMat));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glFrontFace(GL_CCW);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_PATCHES, 0, 16);
}
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
    aspect = static_cast<float>(newWidth) / newHeight;
    glViewport(0, 0, newWidth, newHeight);
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}
int main() {
    if (!glfwInit()) return EXIT_FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 800, "program1202", nullptr, nullptr);
    if (!window) return EXIT_FAILURE;
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return EXIT_FAILURE;
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
    return EXIT_SUCCESS;
}
