#include <cstddef>
#include <thread>

#include <loop.hpp>

static std::string read_next_input() {
    static std::size_t index {0};

    static const std::string commands[] {
        "INIT",
        "SETPARAMETER depth 12",
        "SETPARAMETER max_depth 12",
        "GO",
        "QUIT"
    };

    using namespace std::literals;

    if (commands[index] == "QUIT") {
        std::this_thread::sleep_for(2s);
    }

    return commands[index++];
}

int main() {
    engine::Engine engine;
    return loop::main_loop(engine, read_next_input);
}
