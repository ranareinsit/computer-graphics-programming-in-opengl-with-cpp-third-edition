cmake_minimum_required(VERSION 3.8)
project("program-0602")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)

add_executable(program-0602 main.cpp Torus.cpp Torus.h "Sphere.cpp" "Sphere.h" "Utils.cpp" "Utils.h")

target_link_libraries(program-0602 PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm soil2)

file(COPY vertShader.glsl fragShader.glsl brick1.jpg DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
