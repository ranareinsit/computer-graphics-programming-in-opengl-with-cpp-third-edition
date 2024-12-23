#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
constexpr GLuint numVAOs = 1;
GLuint renderingProgram;
GLuint vao[numVAOs];
void printShaderLog(GLuint shader) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        std::unique_ptr<char[]> log(new char[logLength]);
        GLint written = 0;
        glGetShaderInfoLog(shader, logLength, &written, log.get());
        std::cout << "Shader Info Log: " << log.get() << std::endl;
    }
}
void printProgramLog(GLuint program) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        std::unique_ptr<char[]> log(new char[logLength]);
        GLint written = 0;
        glGetProgramInfoLog(program, logLength, &written, log.get());
        std::cout << "Program Info Log: " << log.get() << std::endl;
    }
}
bool checkOpenGLError() {
    bool foundError = false;
    GLenum glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        std::cout << "glError: " << glErr << std::endl;
        foundError = true;
        glErr = glGetError();
    }
    return foundError;
}
GLuint createShaderProgram() {
    GLint vertCompiled;
    GLint fragCompiled;
    GLint linked;
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
    checkOpenGLError();
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
    if (vertCompiled != GL_TRUE) {
        std::cout << "Vertex compilation failed" << std::endl;
        printShaderLog(vShader);
    }
    glCompileShader(fShader);
    checkOpenGLError();
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &fragCompiled);
    if (fragCompiled != GL_TRUE) {
        std::cout << "Fragment compilation failed" << std::endl;
        printShaderLog(fShader);
    }
    glAttachShader(vfprogram, vShader);
    glAttachShader(vfprogram, fShader);
    glLinkProgram(vfprogram);
    checkOpenGLError();
    glGetProgramiv(vfprogram, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        std::cout << "Linking failed" << std::endl;
        printProgramLog(vfprogram);
    }
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
    auto window = glfwCreateWindow(600, 600, "program 0203", nullptr, nullptr);
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
