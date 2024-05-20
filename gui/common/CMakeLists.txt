cmake_minimum_required(VERSION 3.20)

add_library(common STATIC
    "include/common/board.hpp"
    "include/common/engine.hpp"
    "include/common/fen_string.hpp"
    "include/common/game_state.hpp"
    "include/common/moves_log.hpp"
    "include/common/parameters.hpp"
    "include/common/subprocess.hpp"
    "include/common/wx.hpp"
    "src/board.cpp"
    "src/engine.cpp"
    "src/fen_string.cpp"
    "src/game_state.cpp"
    "src/moves_log.cpp"
    "src/parameters.cpp"
    "src/subprocess.cpp"
)

target_include_directories(common PUBLIC "include")

target_link_libraries(common PRIVATE wxmono)

if(UNIX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_sanitizers(common)
endif()

set_compile_options(common)
set_linux_defines(common)
