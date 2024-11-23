cmake_minimum_required(VERSION 3.20)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

add_subdirectory(extern/googletest)

# Moves unit tests

add_executable(move-generation "src/move_generation.cpp")
target_link_libraries(move-generation PRIVATE checkers-engine-core GTest::gtest_main)
set_compile_options_features(move-generation)

# Moves performance

add_executable(move-generation-perf "src/move_generation_perf.cpp")
target_link_libraries(move-generation-perf PRIVATE checkers-engine-core)
set_compile_options_features(move-generation-perf)

# Search performance

add_executable(search-perf "src/search_perf.cpp")
target_link_libraries(search-perf PRIVATE checkers-engine-core)
set_compile_options_features(search-perf)
