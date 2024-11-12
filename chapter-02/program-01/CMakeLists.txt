cmake_minimum_required(VERSION 3.8)
project("program-0201")

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(program-0201 main.cpp)

target_link_libraries(program-0201 PRIVATE glfw GLEW::GLEW OpenGL::GL)
