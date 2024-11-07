#include "engine.hpp"
#include "loop.hpp"
#include "messages.hpp"

int main() {
    engine::Engine checkers_engine;

    messages::ready();

    return loop::main_loop(checkers_engine);
}
