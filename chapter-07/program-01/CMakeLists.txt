cmake_minimum_required(VERSION 3.8)
project("program-0701")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)

add_executable(program-0701 main.cpp)

target_link_libraries(program-0701 PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm soil2 Common Torus Sphere ImportedModel)

file(COPY BlinnPhongShaders/vertShader.glsl BlinnPhongShaders/fragShader.glsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/BlinnPhongShaders)

file(COPY GouraudShaders/vertShader.glsl GouraudShaders/fragShader.glsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/GouraudShaders)

file(COPY PhongShaders/vertShader.glsl PhongShaders/fragShader.glsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/PhongShaders)
