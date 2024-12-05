#pragma once

#include "game.hpp"
#include "array.hpp"

namespace moves {
    using Moves = array::Array<game::Move, 42>;

    Moves generate_moves(const game::Board& board, game::Player player);
}
