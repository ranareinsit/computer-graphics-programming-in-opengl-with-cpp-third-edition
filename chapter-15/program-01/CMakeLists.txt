cmake_minimum_required(VERSION 3.8)
project("program-1501")
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)
add_executable(program-1501-executable main.cpp)
target_link_libraries(program-1501-executable PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm soil2 Common Torus Sphere ImportedModel)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cubeMap)
configure_file(cubeMap/xn.jpg ${CMAKE_CURRENT_BINARY_DIR}/cubeMap/xn.jpg COPYONLY)
configure_file(cubeMap/xp.jpg ${CMAKE_CURRENT_BINARY_DIR}/cubeMap/xp.jpg COPYONLY)
configure_file(cubeMap/yn.jpg ${CMAKE_CURRENT_BINARY_DIR}/cubeMap/yn.jpg COPYONLY)
configure_file(cubeMap/yp.jpg ${CMAKE_CURRENT_BINARY_DIR}/cubeMap/yp.jpg COPYONLY)
configure_file(cubeMap/zn.jpg ${CMAKE_CURRENT_BINARY_DIR}/cubeMap/zn.jpg COPYONLY)
configure_file(cubeMap/zp.jpg ${CMAKE_CURRENT_BINARY_DIR}/cubeMap/zp.jpg COPYONLY)
configure_file(fragShader.glsl fragShader.glsl COPYONLY)
configure_file(fragCShader.glsl fragCShader.glsl COPYONLY)
configure_file(vertShader.glsl vertShader.glsl COPYONLY)
configure_file(vertCShader.glsl vertCShader.glsl COPYONLY)
