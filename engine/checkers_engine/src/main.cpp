#include <csignal>

#include "engine.hpp"
#include "loop.hpp"
#include "messages.hpp"

int main() {
    if (std::signal(SIGINT, SIG_IGN) == SIG_ERR) {
        return 1;
    }

    engine::Engine checkers_engine;

    messages::ready();

    return loop::main_loop(checkers_engine);
}
