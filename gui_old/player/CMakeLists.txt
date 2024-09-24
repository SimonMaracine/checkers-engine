cmake_minimum_required(VERSION 3.20)

add_executable(player ${WINDOWS_APP}
    "src/main_window.cpp"
    "src/main_window.hpp"
    "src/main.cpp"
)

target_link_libraries(player PRIVATE wxmono common)

target_compile_definitions(player PRIVATE "_CRT_SECURE_NO_WARNINGS")

if(UNIX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    # set_sanitizers(player)
endif()

set_compile_options_features(player)
set_linux_defines(player)
