cmake_minimum_required(VERSION 3.8)
project("program-0402")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)

add_executable(program-0402 main.cpp Utils.cpp)


target_link_libraries(program-0402 PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm soil2)

file(COPY vertShader.glsl fragShader.glsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
