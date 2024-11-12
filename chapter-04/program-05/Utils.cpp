#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Utils.h"

// Constructor for Utils class
Utils::Utils() {}

// Reads the content of a shader file and returns it as a string
std::string Utils::readShaderFile(const char* filePath) {
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open())
        throw std::runtime_error("Unable to open file.");
    std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    fileStream.close();
    return content;
}

// Prints the log information of a shader
void Utils::printShaderLog(GLuint shader) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        std::vector<char> log(len);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        std::cout << "Shader Info Log: " << log.data() << std::endl;
    }
}

// Compiles a shader from a file
GLuint Utils::prepareShader(int shaderType, const char* shaderPath) {
    std::string shaderStr = readShaderFile(shaderPath);
    const char* shaderSrc = shaderStr.c_str();
    GLuint shaderRef = glCreateShader(shaderType);
    glShaderSource(shaderRef, 1, &shaderSrc, nullptr);
    glCompileShader(shaderRef);
    GLint shaderCompiled;
    glGetShaderiv(shaderRef, GL_COMPILE_STATUS, &shaderCompiled);
    if (shaderCompiled != GL_TRUE) {
        std::cerr << "Error compiling shader: " << shaderPath << std::endl;
        printShaderLog(shaderRef);
    }
    return shaderRef;
}

// Links a shader program and verifies the result
int Utils::finalizeShaderProgram(GLuint sprogram) {
    glLinkProgram(sprogram);
    GLint linked;
    glGetProgramiv(sprogram, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        std::cerr << "Error linking program." << std::endl;
    }
    return sprogram;
}

// Creates a shader program from vertex and fragment shader files
GLuint Utils::createShaderProgram(const char* vp, const char* fp) {
    GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
    GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    finalizeShaderProgram(program);
    return program;
}
