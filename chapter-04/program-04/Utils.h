#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Utils
{
private:
	static std::string readShaderFile(const char* filePath);
	static void printShaderLog(GLuint shader);
	static GLuint prepareShader(int shaderTYPE, const char* shaderPath);
	static int finalizeShaderProgram(GLuint sprogram);

public:
	Utils();
	static GLuint createShaderProgram(const char* cp);
	static GLuint createShaderProgram(const char* vp, const char* fp);
};