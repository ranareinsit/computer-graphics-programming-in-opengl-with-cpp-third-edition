#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

constexpr GLuint numVAOs = 1;
GLuint renderingProgram;
GLuint vao[numVAOs];

GLuint createShaderProgram() {
    const char* vshaderSource =
        "#version 430\n"
        "void main(void) {\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "}";

    const char* fshaderSource =
        "#version 430\n"
        "out vec4 color;\n"
        "void main(void) {\n"
        "    color = vec4(0.0, 0.0, 1.0, 1.0);\n"
        "}";

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint vfprogram = glCreateProgram();

    glShaderSource(vShader, 1, &vshaderSource, nullptr);
    glShaderSource(fShader, 1, &fshaderSource, nullptr);
    glCompileShader(vShader);
    glCompileShader(fShader);

    glAttachShader(vfprogram, vShader);
    glAttachShader(vfprogram, fShader);
    glLinkProgram(vfprogram);

    return vfprogram;
}

void init(GLFWwindow* window) {
    renderingProgram = createShaderProgram();
    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window, double currentTime) {
    glUseProgram(renderingProgram);
    glPointSize(30.0f);
    glDrawArrays(GL_POINTS, 0, 1);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    auto window = glfwCreateWindow(600, 600, "program 0202", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
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
