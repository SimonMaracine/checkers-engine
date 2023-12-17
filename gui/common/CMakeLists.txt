cmake_minimum_required(VERSION 3.20)

add_library(common STATIC
    "include/board.hpp"
    "src/board.cpp"
)

target_include_directories(common PUBLIC "include")

target_link_libraries(common PRIVATE wxmono)

set_compile_options(common)
set_linux_defines(common)
