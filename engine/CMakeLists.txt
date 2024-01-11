cmake_minimum_required(VERSION 3.20)

add_executable(checkers-engine
    "src/commands.cpp"
    "src/commands.hpp"
    "src/loop.cpp"
    "src/loop.hpp"
    "src/engine.hpp"
    "src/main.cpp"
    "src/messages.cpp"
    "src/messages.hpp"
)

set_compile_options(checkers-engine)
