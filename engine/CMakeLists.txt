cmake_minimum_required(VERSION 3.20)

add_executable(checkers-engine
    "src/io.cpp"
    "src/io.hpp"
    "src/main.cpp"
)

set_compile_options(checkers-engine)
