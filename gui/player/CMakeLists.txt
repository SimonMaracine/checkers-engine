cmake_minimum_required(VERSION 3.20)

add_executable(player ${WINDOWS_APP}
    "src/application.cpp"
    "src/application.hpp"
    "src/fen_string_dialog.cpp"
    "src/fen_string_dialog.hpp"
    "src/main_window.cpp"
    "src/main_window.hpp"
    "src/subprocess.cpp"
    "src/subprocess.hpp"
)

target_link_libraries(player PRIVATE wxmono common subprocess)

target_compile_definitions(player PRIVATE "_CRT_SECURE_NO_WARNINGS")

set_compile_options(player)
set_linux_defines(player)
