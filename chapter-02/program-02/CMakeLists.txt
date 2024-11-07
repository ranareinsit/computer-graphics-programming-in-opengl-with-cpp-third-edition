cmake_minimum_required(VERSION 3.8)
project("program-02")

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(program-02 main.cpp)

target_link_libraries(program-02 PRIVATE glfw GLEW::GLEW OpenGL::GL)
