cmake_minimum_required(VERSION 3.20)

add_executable(player
    "src/application.cpp"
    "src/application.hpp"
    "src/main_window.cpp"
    "src/main_window.hpp"
)

target_link_libraries(player PRIVATE wx::core wx::base)
