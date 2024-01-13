#pragma once

#include <vector>

#include "game.hpp"

namespace engine {
    struct EngineData {
        /* TODO
            transposition table
            opening book
            parameters
            other
        */

        struct {
            game::Position position;  // Internal position
            std::vector<game::Position> previous_positions;
            std::vector<game::Move> moves_played;
        } game;
    };

    // The commands the engine executes
    // Arguments to these aren't checked
    // May throw errors
    void init(engine::EngineData& data);
    void newgame(engine::EngineData& data);
    void move(engine::EngineData& data, const std::string& move);
    void go(engine::EngineData& data, bool dont_play_move);
    void setparameter(engine::EngineData& data, const std::string& name, const std::string& value);
    void getparameter(engine::EngineData& data, const std::string& name);

    // TODO maybe initialization, finalization
}
