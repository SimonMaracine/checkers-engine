cmake_minimum_required(VERSION 3.20)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

add_subdirectory(extern/googletest)

# Test

add_executable(move-generation "src/move_generation.cpp")

target_link_libraries(move-generation PRIVATE checkers-engine-core GTest::gtest_main)

set_compile_options_features(move-generation)

# Benchmark

add_executable(move-generation-speed "src/move_generation_speed.cpp")

target_link_libraries(move-generation-speed PRIVATE checkers-engine-core)

set_compile_options_features(move-generation-speed)
