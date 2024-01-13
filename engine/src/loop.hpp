#pragma once

#include <string>
#include <array>
#include <cstddef>

#include "engine.hpp"

namespace loop {
    struct InputTokens {
        static constexpr std::size_t MAX {8u};

        std::array<std::string, MAX> tokens {};
        std::size_t count {};
    };

    int main_loop(engine::EngineData& data);
}
