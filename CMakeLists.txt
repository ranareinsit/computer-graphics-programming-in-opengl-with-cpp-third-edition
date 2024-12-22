# CMakeList.txt : CMake project for computer-graphics-programming-in-opengl-with-cpp-third-edition, include source and define
# project specific logic here.
#
set(CMAKE_TOOLCHAIN_FILE "A:/ide/VC/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(CMAKE_CXX_STANDARD 20)

cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("computer-graphics-programming-in-opengl-with-cpp-third-edition")

# common
add_subdirectory(common)
#
add_subdirectory(chapter-02/program-01)
add_subdirectory(chapter-02/program-02)
add_subdirectory(chapter-02/program-03)
add_subdirectory(chapter-02/program-04)
add_subdirectory(chapter-02/program-05)
add_subdirectory(chapter-02/program-06)
#
add_subdirectory(chapter-04/program-01)
add_subdirectory(chapter-04/program-02)
add_subdirectory(chapter-04/program-03)
add_subdirectory(chapter-04/program-04)
add_subdirectory(chapter-04/program-05)
#
add_subdirectory(chapter-05/program-01)
#
add_subdirectory(chapter-06/program-01)
add_subdirectory(chapter-06/program-02)
add_subdirectory(chapter-06/program-03)
#
add_subdirectory(chapter-07/program-01)
#
add_subdirectory(chapter-08/program-01)
add_subdirectory(chapter-08/program-02)
#
add_subdirectory(chapter-09/program-01)
add_subdirectory(chapter-09/program-02)
add_subdirectory(chapter-09/program-03)
#
add_subdirectory(chapter-10/program-01)
add_subdirectory(chapter-10/program-02)
add_subdirectory(chapter-10/program-03)
add_subdirectory(chapter-10/program-04-01)
add_subdirectory(chapter-10/program-04-02)
#
add_subdirectory(chapter-12/program-01)
add_subdirectory(chapter-12/program-02)
add_subdirectory(chapter-12/program-0301)
add_subdirectory(chapter-12/program-0302)
add_subdirectory(chapter-12/program-04)
#
add_subdirectory(chapter-13/program-01-01)
add_subdirectory(chapter-13/program-01-02)
add_subdirectory(chapter-13/program-01-03)
add_subdirectory(chapter-13/program-02)
add_subdirectory(chapter-13/program-03)
add_subdirectory(chapter-13/program-04)
#
add_subdirectory(chapter-14/program-01)
add_subdirectory(chapter-14/program-02)
add_subdirectory(chapter-14/program-03)
add_subdirectory(chapter-14/program-04)
add_subdirectory(chapter-14/program-05)
add_subdirectory(chapter-14/program-06)
add_subdirectory(chapter-14/program-07)
add_subdirectory(chapter-14/program-08)
add_subdirectory(chapter-14/program-09)
#
add_subdirectory(chapter-15/program-01)
add_subdirectory(chapter-15/program-02)
add_subdirectory(chapter-15/program-03)
add_subdirectory(chapter-15/program-04)
add_subdirectory(chapter-15/program-05)
add_subdirectory(chapter-15/program-07)
add_subdirectory(chapter-15/program-08)
#
