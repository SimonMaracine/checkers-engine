#pragma once

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

    // TODO initialization, finalization
}
