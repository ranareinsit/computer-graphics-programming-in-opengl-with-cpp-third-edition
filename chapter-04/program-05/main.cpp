#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stack>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Common.h"

// Constants for Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs)
constexpr int NUM_VAOS = 1;
constexpr int NUM_VBOS = 2;

// Camera position variables
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 0.0f;

// OpenGL-related identifiers
GLuint renderingProgram; // Shader program ID
GLuint vao[NUM_VAOS];    // Array for VAO IDs
GLuint vbo[NUM_VBOS];    // Array for VBO IDs

// Uniform location identifiers
GLuint mvLoc = 0, projLoc = 0; // Model-View and Projection Matrix locations

// Window dimensions and aspect ratio
int width = 0, height = 0;
float aspect = 0.0f;

// Transformation matrices
glm::mat4 pMat;   // Perspective matrix
glm::mat4 vMat;   // View matrix
glm::mat4 mMat;   // Model matrix
glm::mat4 mvMat;  // Model-View matrix

// Stack for matrix transformations
std::stack<glm::mat4> mvStack;


static const float vertexPositions[108] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f
};

static const float pyramidPositions[54] = {
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, -1.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f
};

// Sets up vertex data for rendering
void setupVertices() {
	// Generate a Vertex Array Object (VAO) and bind it
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// Generate Vertex Buffer Objects (VBOs)
	glGenBuffers(NUM_VBOS, vbo);

	// Bind and upload vertex positions for the cube to the first VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);

	// Bind and upload vertex positions for the pyramid to the second VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidPositions), pyramidPositions, GL_STATIC_DRAW);
}

// Initializes the OpenGL context and program
void init(GLFWwindow* window) {
	// Create and compile the shader program from GLSL source files
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");

	// Get the framebuffer size and calculate the aspect ratio
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;

	// Set up the perspective projection matrix
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees

	// Initialize the camera position
	cameraX = 0.0f; cameraY = 0.0f; cameraZ = 12.0f;

	// Set up vertex buffers and data
	setupVertices();
}


// Renders the scene, handling transformations and drawing objects
void display(GLFWwindow* window, double currentTime) {
	// Clear the depth and color buffers
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0); // Set background color to black
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Activate the shader program
	glUseProgram(renderingProgram);

	// Get uniform locations for transformation matrices
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

	// Setup the view matrix to position the camera
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mvStack.push(vMat);

	// Send the projection matrix to the shader
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	//  Pyramid (Sun)
	mvStack.push(mvStack.top()); // Save the view matrix
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // Center position
	mvStack.push(mvStack.top()); // Save position
	mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(1.0, 0.0, 0.0)); // Rotate
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // Pyramid vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 18); // Draw the pyramid
	mvStack.pop();

	//  Cube (Planet)
	mvStack.push(mvStack.top()); // Save the view matrix
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime) * 4.0, 0.0f, cos((float)currentTime) * 4.0)); // Orbit
	mvStack.push(mvStack.top()); // Save position
	mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 1.0, 0.0)); // Rotate
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // Cube vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 36); // Draw the cube
	mvStack.pop();

	//  Smaller Cube (Moon)
	mvStack.push(mvStack.top()); // Save planet position
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime) * 2.0, cos((float)currentTime) * 2.0)); // Orbit around planet
	mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 0.0, 1.0)); // Rotate
	mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f)); // Scale down
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // Cube vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CCW);
	glDrawArrays(GL_TRIANGLES, 0, 36); // Draw the moon
	mvStack.pop(); mvStack.pop(); mvStack.pop(); // Pop moon, planet, and sun transformations

	mvStack.pop(); // Pop the view matrix
}


// Callback function to handle window resizing
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
	// Update the aspect ratio based on new window dimensions
	aspect = (float)newWidth / (float)newHeight;

	// Set the new viewport to match the updated window size
	glViewport(0, 0, newWidth, newHeight);

	// Recalculate the projection matrix with the updated aspect ratio
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}


int main(void) {
	// Initialize GLFW
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	// Set OpenGL version hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Create a windowed GLFW window
	GLFWwindow* window = glfwCreateWindow(600, 600, "chapter-04 program-05", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Make the OpenGL context current
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		exit(EXIT_FAILURE);
	}

	// Enable V-Sync
	glfwSwapInterval(1);

	// Set window size callback function
	glfwSetWindowSizeCallback(window, window_size_callback);

	// Initialize the rendering program and resources
	init(window);

	// Main rendering loop
	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup and exit
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
