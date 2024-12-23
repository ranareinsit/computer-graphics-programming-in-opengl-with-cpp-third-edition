#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/euler_angles.hpp>
#include "Common.h"
using namespace std;
Utils::Utils() {}
void Utils::displayComputeShaderLimits() {
	int work_grp_cnt[3];
	int work_grp_siz[3];
	int work_grp_inv;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_siz[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_siz[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_siz[2]);
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("maximum number of workgroups is: %i  %i  %i\n", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);
	printf("maximum size of workgroups is: %i  %i  %i\n", work_grp_siz[0], work_grp_siz[1], work_grp_siz[2]);
	printf("max local work group invocations %i\n", work_grp_inv);
}
string Utils::readShaderFile(const char* filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	if (!fileStream.is_open()) {

		throw "Unable to open file.";
	}
	string line = "";
	while (!fileStream.eof()) {

		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}
bool Utils::checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {

		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}
void Utils::printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {

		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}
GLuint Utils::prepareShader(int shaderTYPE, const char* shaderPath) {
	GLint shaderCompiled;
	string shaderStr = readShaderFile(shaderPath);
	const char* shaderSrc = shaderStr.c_str();
	GLuint shaderRef = glCreateShader(shaderTYPE);
	if (shaderRef == 0 || shaderRef == GL_INVALID_ENUM) {

		printf("Error: Could not create shader \"%s\" of type:%d\n", shaderPath, shaderTYPE);
		return 0;
	}
	glShaderSource(shaderRef, 1, &shaderSrc, NULL);
	glCompileShader(shaderRef);
	checkOpenGLError();
	glGetShaderiv(shaderRef, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE) {

		if (shaderTYPE == GL_VERTEX_SHADER) cout << "Vertex ";
		if (shaderTYPE == GL_TESS_CONTROL_SHADER) cout << "Tess Control ";
		if (shaderTYPE == GL_TESS_EVALUATION_SHADER) cout << "Tess Eval ";
		if (shaderTYPE == GL_GEOMETRY_SHADER) cout << "Geometry ";
		if (shaderTYPE == GL_FRAGMENT_SHADER) cout << "Fragment ";
		if (shaderTYPE == GL_COMPUTE_SHADER) cout << "Compute ";
		cout << "shader compilation error for shader: '" << shaderPath << "'." << endl;
		printShaderLog(shaderRef);
	}
	return shaderRef;
}
void Utils::printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}
int Utils::finalizeShaderProgram(GLuint sprogram) {
	GLint linked;
	glLinkProgram(sprogram);
	checkOpenGLError();
	glGetProgramiv(sprogram, GL_LINK_STATUS, &linked);
	if (linked != 1) {

		cout << "linking failed" << endl;
		printProgramLog(sprogram);
	}
	return sprogram;
}
GLuint Utils::createShaderProgram(const char* cp) {
	GLuint cShader = prepareShader(GL_COMPUTE_SHADER, cp);
	GLuint cprogram = glCreateProgram();
	glAttachShader(cprogram, cShader);
	finalizeShaderProgram(cprogram);
	return cprogram;
}
GLuint Utils::createShaderProgram(const char* vp, const char* fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vfprogram = glCreateProgram();
	glAttachShader(vfprogram, vShader);
	glAttachShader(vfprogram, fShader);
	finalizeShaderProgram(vfprogram);
	return vfprogram;
}
GLuint Utils::createShaderProgram(const char* vp, const char* gp, const char* fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint gShader = prepareShader(GL_GEOMETRY_SHADER, gp);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vgfprogram = glCreateProgram();
	glAttachShader(vgfprogram, vShader);
	glAttachShader(vgfprogram, gShader);
	glAttachShader(vgfprogram, fShader);
	finalizeShaderProgram(vgfprogram);
	return vgfprogram;
}
GLuint Utils::createShaderProgram(const char* vp, const char* tCS, const char* tES, const char* fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint tcShader = prepareShader(GL_TESS_CONTROL_SHADER, tCS);
	GLuint teShader = prepareShader(GL_TESS_EVALUATION_SHADER, tES);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vtfprogram = glCreateProgram();
	glAttachShader(vtfprogram, vShader);
	glAttachShader(vtfprogram, tcShader);
	glAttachShader(vtfprogram, teShader);
	glAttachShader(vtfprogram, fShader);
	finalizeShaderProgram(vtfprogram);
	return vtfprogram;
}
GLuint Utils::createShaderProgram(const char* vp, const char* tCS, const char* tES, char* gp, const char* fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint tcShader = prepareShader(GL_TESS_CONTROL_SHADER, tCS);
	GLuint teShader = prepareShader(GL_TESS_EVALUATION_SHADER, tES);
	GLuint gShader = prepareShader(GL_GEOMETRY_SHADER, gp);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vtgfprogram = glCreateProgram();
	glAttachShader(vtgfprogram, vShader);
	glAttachShader(vtgfprogram, tcShader);
	glAttachShader(vtgfprogram, teShader);
	glAttachShader(vtgfprogram, gShader);
	glAttachShader(vtgfprogram, fShader);
	finalizeShaderProgram(vtgfprogram);
	return vtgfprogram;
}
