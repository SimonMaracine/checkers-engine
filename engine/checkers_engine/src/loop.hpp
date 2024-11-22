#pragma once

#ifdef CHECKERS_ENGINE_TEST
    #include <string>
#endif

#include "engine.hpp"

namespace loop {
    int main_loop(engine::Engine& engine);

#ifdef CHECKERS_ENGINE_TEST
    namespace test {
        int main_loop(engine::Engine& engine, std::string(*read_next_input)());
    }
#endif
}
