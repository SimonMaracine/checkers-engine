cmake_minimum_required(VERSION 3.20)

add_executable(tester ${WINDOWS_APP}
    "src/main.cpp"
)

target_link_libraries(tester PRIVATE wxmono common)

set_compile_options(tester)
set_linux_defines(tester)
