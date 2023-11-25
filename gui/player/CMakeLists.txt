cmake_minimum_required(VERSION 3.20)

if(WIN32)
    set(WINDOWS_APP WIN32)
endif()

add_executable(player ${WINDOWS_APP}
    "src/application.cpp"
    "src/application.hpp"
    "src/board.cpp"
    "src/board.hpp"
    "src/fen_string_dialog.cpp"
    "src/fen_string_dialog.hpp"
    "src/main_window.cpp"
    "src/main_window.hpp"
)

target_link_libraries(player PRIVATE wxmono)

set_compile_options(player)
