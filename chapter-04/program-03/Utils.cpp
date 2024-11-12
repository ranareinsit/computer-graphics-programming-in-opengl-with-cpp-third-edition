#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Utils.h"

Utils::Utils() {}

std::string Utils::readShaderFile(const char* filePath) {
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) throw "Unable to open file.";
    std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    fileStream.close();
    return content;
}

void Utils::printShaderLog(GLuint shader) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        std::vector<char> log(len);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        std::cout << "Shader Info Log: " << log.data() << std::endl;
    }
}

GLuint Utils::prepareShader(int shaderTYPE, const char* shaderPath) {
    std::string shaderStr = readShaderFile(shaderPath);
    const char* shaderSrc = shaderStr.c_str();
    GLuint shaderRef = glCreateShader(shaderTYPE);
    glShaderSource(shaderRef, 1, &shaderSrc, nullptr);
    glCompileShader(shaderRef);
    GLint shaderCompiled;
    glGetShaderiv(shaderRef, GL_COMPILE_STATUS, &shaderCompiled);
    if (shaderCompiled != GL_TRUE) {
        printShaderLog(shaderRef);
    }
    return shaderRef;
}

int Utils::finalizeShaderProgram(GLuint sprogram) {
    glLinkProgram(sprogram);
    GLint linked;
    glGetProgramiv(sprogram, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        std::cout << "Linking failed" << std::endl;
    }
    return sprogram;
}

GLuint Utils::createShaderProgram(const char* vp, const char* fp) {
    GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
    GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    finalizeShaderProgram(program);
    return program;
}
