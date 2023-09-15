cmake_minimum_required(VERSION 3.20)

add_executable(tester "src/main.cpp")

target_link_libraries(tester PRIVATE fltk)
target_include_directories(tester PRIVATE  # This is what the library is doing anyway :P
    "../extern/fltk"
    "${FLTK_BINARY_DIR}"
)
