cmake_minimum_required(VERSION 3.8)
project("program-100401")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)

add_executable(program-100401-executable main.cpp)

target_link_libraries(program-100401-executable PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm soil2 Common Torus Sphere ImportedModel)

configure_file(fragShader.glsl fragShader.glsl COPYONLY)
configure_file(vertShader.glsl vertShader.glsl COPYONLY)
configure_file(height.jpg height.jpg COPYONLY)
configure_file(heightTexture.jpg heightTexture.jpg COPYONLY)
configure_file(grid.obj grid.obj COPYONLY)
