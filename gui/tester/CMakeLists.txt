cmake_minimum_required(VERSION 3.20)

add_executable(tester
    "src/board.cpp"
    "src/board.hpp"
    "src/controls.cpp"
    "src/controls.hpp"
    "src/main.cpp"
    "src/window.cpp"
    "src/window.hpp"
)

target_link_libraries(tester PRIVATE fltk)

target_include_directories(tester PRIVATE  # This is what the library is doing anyway :P
    "../extern/fltk"
    "${FLTK_BINARY_DIR}"
)
