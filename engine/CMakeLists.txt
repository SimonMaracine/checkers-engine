cmake_minimum_required(VERSION 3.20)

add_executable(checkers-engine
    "src/commands.cpp"
    "src/commands.hpp"
    "src/engine.cpp"
    "src/engine.hpp"
    "src/game.cpp"
    "src/game.hpp"
    "src/loop.cpp"
    "src/loop.hpp"
    "src/main.cpp"
    "src/messages.cpp"
    "src/messages.hpp"
    "src/moves.cpp"
    "src/moves.hpp"
)

set_compile_options(checkers-engine)
