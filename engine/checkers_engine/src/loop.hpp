#pragma once

#include <string>

#include "engine.hpp"

namespace loop {
    std::string read_input();
    int main_loop(engine::Engine& engine, std::string(*read_input)());
}
