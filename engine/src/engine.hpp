#pragma once

#include <string>

#include "game.hpp"

namespace engine {
    struct EngineData {
        /* TODO
            transposition table
            opening book
            parameters
            other
        */

       game::Position position;  // Internal position
    //    unsigned int plies {0u};
    //    unsigned int plies_without_advancement {0u};
    };

    bool set_position(EngineData& data, const std::string& fen_string);
    void reset_position(EngineData& data);

    // TODO initialization, finalization
}
