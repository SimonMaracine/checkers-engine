#include "engine.hpp"
#include "loop.hpp"
#include "messages.hpp"

int main() {
    engine::EngineData data;

    messages::ready();

    return loop::main_loop(data);
}
