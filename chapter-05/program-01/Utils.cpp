#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/soil2.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Utils {
public:
    Utils() = default;

    static void displayComputeShaderLimits();
    static std::string readShaderFile(const char* filePath);
    static bool checkOpenGLError();
    static void printShaderLog(GLuint shader);
    static GLuint prepareShader(int shaderType, const char* shaderPath);
    static void printProgramLog(GLuint program);
    static GLuint finalizeShaderProgram(GLuint program);
    static GLuint createShaderProgram(const char* computeShaderPath);
    static GLuint createShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath);
    static GLuint createShaderProgram(const char* vertexShaderPath, const char* geometryShaderPath, const char* fragmentShaderPath);
    static GLuint createShaderProgram(const char* vertexShaderPath, const char* tessControlShaderPath, const char* tessEvalShaderPath, const char* fragmentShaderPath);
    static GLuint createShaderProgram(const char* vertexShaderPath, const char* tessControlShaderPath, const char* tessEvalShaderPath, const char* geometryShaderPath, const char* fragmentShaderPath);
    static GLuint loadCubeMap(const char* mapDir);
    static GLuint loadTexture(const char* texturePath);
};

void Utils::displayComputeShaderLimits() {
    int workGrpCnt[3], workGrpSize[3], workGrpInv;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGrpCnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGrpCnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGrpCnt[2]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGrpSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGrpSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGrpSize[2]);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGrpInv);
    printf("Max workgroups: %i %i %i\nMax workgroup size: %i %i %i\nMax local invocations: %i\n",
        workGrpCnt[0], workGrpCnt[1], workGrpCnt[2], workGrpSize[0], workGrpSize[1], workGrpSize[2], workGrpInv);
}

std::string Utils::readShaderFile(const char* filePath) {
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) throw std::runtime_error("Unable to open file.");
    return std::string((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
}

bool Utils::checkOpenGLError() {
    bool foundError = false;
    while (GLenum err = glGetError()) {
        std::cerr << "GL Error: " << err << '\n';
        foundError = true;
    }
    return foundError;
}

void Utils::printShaderLog(GLuint shader) {
    int len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        std::vector<char> log(len);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        std::cerr << "Shader Log: " << log.data() << '\n';
    }
}

GLuint Utils::prepareShader(int shaderType, const char* shaderPath) {
    std::string shaderStr = readShaderFile(shaderPath);
    const char* shaderSrc = shaderStr.c_str();
    GLuint shaderRef = glCreateShader(shaderType);
    if (!shaderRef) throw std::runtime_error("Shader creation failed.");
    glShaderSource(shaderRef, 1, &shaderSrc, nullptr);
    glCompileShader(shaderRef);
    if (checkOpenGLError()) throw std::runtime_error("Shader compilation error.");
    return shaderRef;
}

void Utils::printProgramLog(GLuint program) {
    int len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        std::vector<char> log(len);
        glGetProgramInfoLog(program, len, nullptr, log.data());
        std::cerr << "Program Log: " << log.data() << '\n';
    }
}

GLuint Utils::finalizeShaderProgram(GLuint program) {
    glLinkProgram(program);
    if (checkOpenGLError()) throw std::runtime_error("Program linking failed.");
    return program;
}

GLuint Utils::createShaderProgram(const char* computeShaderPath) {
    GLuint cShader = prepareShader(GL_COMPUTE_SHADER, computeShaderPath);
    GLuint program = glCreateProgram();
    glAttachShader(program, cShader);
    return finalizeShaderProgram(program);
}

GLuint Utils::createShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
    GLuint vShader = prepareShader(GL_VERTEX_SHADER, vertexShaderPath);
    GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fragmentShaderPath);
    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    return finalizeShaderProgram(program);
}

GLuint Utils::loadCubeMap(const char* mapDir) {
    GLuint textureRef = SOIL_load_OGL_cubemap(
        (std::string(mapDir) + "/xp.jpg").c_str(), (std::string(mapDir) + "/xn.jpg").c_str(),
        (std::string(mapDir) + "/yp.jpg").c_str(), (std::string(mapDir) + "/yn.jpg").c_str(),
        (std::string(mapDir) + "/zp.jpg").c_str(), (std::string(mapDir) + "/zn.jpg").c_str(),
        SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS
    );
    if (!textureRef) throw std::runtime_error("Cube map loading failed.");
    return textureRef;
}

GLuint Utils::loadTexture(const char* texturePath) {
    GLuint textureRef = SOIL_load_OGL_texture(texturePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if (!textureRef) throw std::runtime_error("Texture loading failed.");
    glBindTexture(GL_TEXTURE_2D, textureRef);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    return textureRef;
}
