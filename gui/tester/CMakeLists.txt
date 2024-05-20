cmake_minimum_required(VERSION 3.20)

add_executable(tester ${WINDOWS_APP}
    "src/main_window.cpp"
    "src/main_window.hpp"
    "src/main.cpp"
)

target_link_libraries(tester PRIVATE wxmono common)

target_compile_definitions(tester PRIVATE "_CRT_SECURE_NO_WARNINGS")

if(UNIX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_sanitizers(tester)
endif()

set_compile_options(tester)
set_linux_defines(tester)
