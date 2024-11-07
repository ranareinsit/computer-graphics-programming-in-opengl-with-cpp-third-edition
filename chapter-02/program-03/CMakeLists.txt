cmake_minimum_required(VERSION 3.8)
project("program-03")

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(program-03 main.cpp)

target_link_libraries(program-03 PRIVATE glfw GLEW::GLEW OpenGL::GL)
