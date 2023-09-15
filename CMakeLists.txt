cmake_minimum_required(VERSION 3.20)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

project("checkers-artificial-intelligence" LANGUAGES CXX)

add_subdirectory(library)
add_subdirectory(gui)
add_subdirectory(tests)
