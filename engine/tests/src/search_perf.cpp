#include <cstddef>

#include <loop.hpp>

static std::string read_next_input() {
    static std::size_t index {0};

    static const std::string commands[] {
        "INIT",
        "SETPARAMETER depth 15",
        "GO",
        "QUIT"
    };

    return commands[index++];
}

int main() {
    engine::Engine engine;
    return loop::main_loop(engine, read_next_input);
}
