#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>
#include <ranges>

namespace fs = std::filesystem;

#define numVAOs 1

GLuint renderingProgram;
GLuint vao[numVAOs];

// Read file into a string
std::string readFile(const fs::path& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        exit(EXIT_FAILURE);
    }
    return { std::istreambuf_iterator<char>{fileStream}, std::istreambuf_iterator<char>{} };
}

// Check shader compilation errors
void checkShaderCompilation(GLuint shader) {
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength, '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::cerr << "Shader compilation failed:\n" << log << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Check program linking errors
void checkProgramLinking(GLuint program) {
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength, '\0');
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        std::cerr << "Program linking failed:\n" << log << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Create the shader program
GLuint createShaderProgram() {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();

    // Read shaders from file
    auto vertShaderStr = readFile("vertShader.glsl");
    auto fragShaderStr = readFile("fragShader.glsl");

    const char* vertShaderSrc = vertShaderStr.c_str();
    const char* fragShaderSrc = fragShaderStr.c_str();

    glShaderSource(vShader, 1, &vertShaderSrc, nullptr);
    glShaderSource(fShader, 1, &fragShaderSrc, nullptr);

    glCompileShader(vShader);
    checkShaderCompilation(vShader);

    glCompileShader(fShader);
    checkShaderCompilation(fShader);

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);

    glLinkProgram(program);
    checkProgramLinking(program);

    return program;
}

// Initialize OpenGL
void init(GLFWwindow* window) {
    renderingProgram = createShaderProgram();
    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
}

// Display callback
void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);
    glPointSize(30.0f);
    glDrawArrays(GL_POINTS, 0, 1);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(600, 600, "program 0204", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSwapInterval(1);

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
