#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "game.hpp"
#include "parameters.hpp"
#include "evaluation.hpp"

namespace messages {
    // Messages the engine sends to GUI
    void ready();
    void bestmove(const std::optional<game::Move>& move);
    void parameters(const std::unordered_map<std::string, parameters::Parameter>& parameters);
    void parameter(const std::string& name, const parameters::Parameter& value);
    void info(unsigned int nodes, unsigned int transpositions, unsigned int depth, evaluation::Eval eval, double time);
    void name();
}
