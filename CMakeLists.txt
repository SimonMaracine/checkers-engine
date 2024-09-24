cmake_minimum_required(VERSION 3.20)

project("checkers-artificial-intelligence" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

function(set_compile_options_features target)
    if(UNIX)
        target_compile_options(${target} PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wconversion")
    elseif(MSVC)
        target_compile_options(${target} PRIVATE "/W4")
    else()
        message(WARNING "Warnings are not enabled")
    endif()

    target_compile_features(${target} PRIVATE cxx_std_17)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()

add_subdirectory(engine)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
