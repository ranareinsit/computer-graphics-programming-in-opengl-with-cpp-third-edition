#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include <cstdlib>
using namespace std;

#define numVAOs 1

GLuint renderingProgram;
GLuint vao[numVAOs];

static GLuint createShaderProgram() {
	const char* vshaderSource =
		"#version 430 \n"
		"void main(void) \n"
		"{gl_Position = vec4(0.0,0.0,0.0,1.0);}";
	const char* fshaderSource =
		"#version 430 \n"
		"out vec4 color; \n"
		"uniform vec3 boxColor; \n"
		"void main(void) {  if (gl_FragCoord.x < 295) color = vec4(boxColor, 1.0); else color = vec4(0.0, 0.0, 1.0, 1.0);  }";

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vShader, 1, &vshaderSource, NULL);
	glShaderSource(fShader, 1, &fshaderSource, NULL);
	glCompileShader(vShader);
	glCompileShader(fShader);
	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram);
	return vfProgram;
}

static float lerp(float firstFloat, float secondFloat, float by) {
	return firstFloat * (1 - by) + secondFloat * by;
}

static float clamp(float value, float min, float max) {
	return value < min ? min : value > max ? max : value;
}

static float random() {
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

int main() {
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(256, 256, "Color-Changing Box", NULL, NULL);
	glfwMakeContextCurrent(window);
	glewInit();
	renderingProgram = createShaderProgram();
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);

	array<float, 3> rgb_array = { 0.0f, 0.0f, 0.0f };
	array<int, 3> color_vector = { 1, 1, 1 };
	array<float, 3> color_delta = { 0.0001f, 0.0001f, 0.0001f };

	float max = 1.0f;
	float min = 0.0f;
	float fact = 0.0001f;

	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		//
		for (size_t i = 0; i < rgb_array.size(); i++) {
			if (rgb_array[i] >= max || rgb_array[i] <= min) {
				color_vector[i] = -color_vector[i];
				color_delta[i] = clamp(random() * fact, 0.00005f, 0.0015f);
			}
			rgb_array[i] = clamp(rgb_array[i] + (color_delta[i] * color_vector[i]), min, max);
		}
		//
		glClearColor(rgb_array[0], rgb_array[1], rgb_array[2], 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		//
		glUseProgram(renderingProgram);
		GLuint colorLoc = glGetUniformLocation(renderingProgram, "boxColor");
		glUniform3f(colorLoc, 1.0f - rgb_array[0], 1.0f - rgb_array[1], 1.0f - rgb_array[2]);
		glPointSize(128.0f);
		glDrawArrays(GL_POINTS, 0, 1);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
