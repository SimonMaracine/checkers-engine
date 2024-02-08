#pragma once

#include <string>

#include "game.hpp"
#include "engine.hpp"

namespace messages {
    // Messages the engine sends to GUI
    void warning(const std::string& message = "");
    void bestmove(const game::Move& move);
    void parameter(const std::string& name, const engine::Param& value);
}
