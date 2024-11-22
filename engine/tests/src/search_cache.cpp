#include <cstddef>

#define CHECKERS_ENGINE_TEST
#include <loop.hpp>

static std::string next_input() {
    static std::size_t index {0};

    static const std::string commands[] {
        "INIT",
        "SETPARAMETER depth 11",
        "GO",
        "QUIT"
    };

    return commands[index++];
}

int main() {
    engine::Engine engine;
    return loop::test::main_loop(engine, next_input);
}
