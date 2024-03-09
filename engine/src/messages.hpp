#pragma once

#include <string>
#include <unordered_map>

#include "game.hpp"
#include "engine.hpp"

namespace messages {
    // Messages the engine sends to GUI
    void bestmove(const game::Move& move);
    void parameters(const std::unordered_map<std::string, engine::Param>& parameters);
    void parameter(const std::string& name, const engine::Param& value);
}
