cmake_minimum_required(VERSION 3.20)

add_executable(checkers-engine
    "src/commands.cpp"
    "src/commands.hpp"
    "src/engine.cpp"
    "src/engine.hpp"
    "src/error.hpp"
    "src/evaluation.cpp"
    "src/evaluation.hpp"
    "src/game.cpp"
    "src/game.hpp"
    "src/input_tokens.hpp"
    "src/loop.cpp"
    "src/loop.hpp"
    "src/main.cpp"
    "src/messages.cpp"
    "src/messages.hpp"
    "src/moves.cpp"
    "src/moves.hpp"
    "src/search_node.cpp"
    "src/search_node.hpp"
    "src/search.cpp"
    "src/search.hpp"
)

set_compile_options(checkers-engine)

if(UNIX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(checkers-engine PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-g" "-fno-omit-frame-pointer" "-libasan")
    target_link_options(checkers-engine PRIVATE "-fsanitize=address" "-fsanitize=undefined")

    message(STATUS "Building engine with sanitizers")
endif()
