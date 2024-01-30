#pragma once

#include <string>

#include "game.hpp"

namespace messages {
    // Messages the engine sends to GUI
    void warning(const std::string& message = "");
    void bestmove(const game::Move& move);
}
