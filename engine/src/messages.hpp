#pragma once

#include <string>
#include <optional>

#include "game.hpp"

namespace messages {
    // Messages the engine sends to GUI
    void warning(const std::optional<std::string>& message = std::nullopt);
    void bestmove(const game::Move& move);
}
