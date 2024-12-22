cmake_minimum_required(VERSION 3.8)
project("program-1401")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)

add_executable(program-1401-executable main.cpp)

target_link_libraries(program-1401-executable PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm soil2 Common Torus Sphere ImportedModel)

configure_file(fragShader.glsl fragShader.glsl COPYONLY)
configure_file(vertShader.glsl vertShader.glsl COPYONLY)

configure_file(grid.obj grid.obj COPYONLY)

configure_file(DisplacementMap.png DisplacementMap.png COPYONLY)
configure_file(NormalMap.png NormalMap.png COPYONLY)
configure_file(SpecularMap.png SpecularMap.png COPYONLY)
configure_file(texture.jpg texture.jpg COPYONLY)