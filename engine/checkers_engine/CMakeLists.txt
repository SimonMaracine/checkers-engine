cmake_minimum_required(VERSION 3.20)

# https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html

# Core library

add_library(checkers-engine-core STATIC
    "src/array.hpp"
    "src/commands.cpp"
    "src/commands.hpp"
    "src/engine.cpp"
    "src/engine.hpp"
    "src/error.hpp"
    "src/evaluation.cpp"
    "src/evaluation.hpp"
    "src/game.cpp"
    "src/game.hpp"
    "src/loop.cpp"
    "src/loop.hpp"
    "src/messages.cpp"
    "src/messages.hpp"
    "src/moves.cpp"
    "src/moves.hpp"
    "src/parameters.hpp"
    "src/search.cpp"
    "src/search.hpp"
    "src/transposition_table.cpp"
    "src/transposition_table.hpp"
    "src/utils.hpp"
    "src/zobrist.cpp"
    "src/zobrist.hpp"
)

target_include_directories(checkers-engine-core PUBLIC "src")

set_compile_options_features(checkers-engine-core)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(checkers-engine-core PRIVATE "NDEBUG")
endif()

target_compile_definitions(checkers-engine-core PRIVATE "_CRT_SECURE_NO_WARNINGS")

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND UNIX AND USE_SANITIZERS)
    target_compile_options(checkers-engine-core PUBLIC "-fsanitize=address" "-fsanitize=undefined" "-g" "-fno-omit-frame-pointer")
    target_link_options(checkers-engine-core PUBLIC "-fsanitize=address" "-fsanitize=undefined")

    message(STATUS "Building engine with sanitizers")
endif()

set_target_properties(checkers-engine-core PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)

# Executable

add_executable(checkers-engine "src/main.cpp")

target_link_libraries(checkers-engine PRIVATE checkers-engine-core)

set_compile_options_features(checkers-engine)

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND UNIX)
    target_link_options(checkers-engine PRIVATE "-s")
endif()

set_target_properties(checkers-engine PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
