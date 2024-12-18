cmake_minimum_required(VERSION 3.8)
project("program-0701")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)

add_executable(program-0701 main.cpp)

target_link_libraries(program-0701 PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm soil2 Common Torus Sphere ImportedModel)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/BlinnPhongShaders)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GouraudShaders)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/PhongShaders)

configure_file(BlinnPhongShaders/vertShader.glsl BlinnPhongShaders/vertShader.glsl COPYONLY)
configure_file(BlinnPhongShaders/fragShader.glsl BlinnPhongShaders/fragShader.glsl COPYONLY)
configure_file(GouraudShaders/vertShader.glsl GouraudShaders/vertShader.glsl COPYONLY)
configure_file(GouraudShaders/fragShader.glsl GouraudShaders/fragShader.glsl COPYONLY)
configure_file(PhongShaders/vertShader.glsl PhongShaders/vertShader.glsl COPYONLY)
configure_file(PhongShaders/fragShader.glsl PhongShaders/fragShader.glsl COPYONLY)
