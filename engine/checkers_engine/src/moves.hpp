#pragma once

#include <vector>

#include "game.hpp"

namespace moves {
    std::vector<game::Move> generate_moves(const game::Board& board, game::Player player);
}
