cmake_minimum_required(VERSION 3.20)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

add_subdirectory(extern/googletest)

# Test

add_executable(test-engine "move_generation.cpp")

target_link_libraries(test-engine PRIVATE checkers-engine-core GTest::gtest_main)

set_compile_options(test-engine)

# Benchmark

add_executable(test-engine-speed "move_generation_speed.cpp")

target_link_libraries(test-engine-speed PRIVATE checkers-engine-core)

set_compile_options(test-engine-speed)
