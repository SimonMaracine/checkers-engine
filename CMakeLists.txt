cmake_minimum_required(VERSION 3.20)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

project("checkers-artificial-intelligence" LANGUAGES CXX)

function(set_compile_options TARGET)
    if(UNIX)
        target_compile_options(${TARGET} PRIVATE "-Wall" "-Wextra" "-Wpedantic")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${TARGET} PRIVATE "/W3")
    else()
        message(WARNING "Compiler is not MSVC or platform is not supported, warnings are not enabled")
    endif()

    target_compile_features(${TARGET} PRIVATE cxx_std_17)
    set_target_properties(${TARGET} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
# elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
#     set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
# endif()

add_subdirectory(library)
add_subdirectory(gui)
add_subdirectory(tests)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)