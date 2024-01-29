#pragma once

#include <vector>

#include "game.hpp"

namespace search {
    class Search {
    public:
        game::Move search(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );
    private:

    };
}
