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
    void info(
        int nodes,
        int transpositions,
        int depth,
        evaluation::Eval eval,
        double time,
        const game::Move* pv_moves,
        int pv_size
    );
    void name();
    void board(const game::Position& position);
}
