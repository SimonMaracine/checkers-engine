cmake_minimum_required(VERSION 3.20)

add_executable(checkers-engine
    "src/commands.cpp"
    "src/commands.hpp"
    "src/engine.cpp"
    "src/engine.hpp"
    "src/error.hpp"
    "src/game.cpp"
    "src/game.hpp"
    "src/loop.cpp"
    "src/loop.hpp"
    "src/main.cpp"
    "src/messages.cpp"
    "src/messages.hpp"
    "src/moves.cpp"
    "src/moves.hpp"
    "src/search.cpp"
    "src/search.hpp"
)

set_compile_options(checkers-engine)
