#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"

namespace messages {
    // Messages the engine sends to GUI
    void ready();
    void bestmove(const std::optional<game::Move>& move);
    void parameters(const std::unordered_map<std::string, engine::Parameter>& parameters);
    void parameter(const std::string& name, const engine::Parameter& value);
    void info(unsigned int nodes, evaluation::Eval eval, double time);
    void name();
}
