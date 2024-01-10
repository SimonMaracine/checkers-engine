cmake_minimum_required(VERSION 3.20)

add_executable(checkers-engine
    "src/loop.cpp"
    "src/loop.hpp"
    "src/engine.hpp"
    "src/main.cpp"
)

set_compile_options(checkers-engine)
