cmake_minimum_required(VERSION 3.20)

if(WIN32)
    set(WINDOWS_APP WIN32)
endif()

add_executable(tester ${WINDOWS_APP}
    "src/main.cpp"
)

target_link_libraries(tester PRIVATE wxmono)

set_compile_options(tester)
