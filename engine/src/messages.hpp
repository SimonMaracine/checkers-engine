#pragma once

#include <string>
#include <optional>

#include "game.hpp"

namespace messages {
    void warning(const std::optional<std::string>& message = std::nullopt);
    void bestmove(const game::Move& move);
}
