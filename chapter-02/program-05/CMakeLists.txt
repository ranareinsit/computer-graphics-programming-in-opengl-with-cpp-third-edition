cmake_minimum_required(VERSION 3.8)
project("program-0205")

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(program-0205 main.cpp)

target_link_libraries(program-0205 PRIVATE glfw GLEW::GLEW OpenGL::GL)

# Copy shaders to the build directory
file(COPY vertShader.glsl fragShader.glsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
