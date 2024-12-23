#pragma once

#include "game.hpp"
#include "array.hpp"

namespace moves {
    using Moves = array::Array<game::Move, 35>;

    Moves generate_moves(const game::Position& position) noexcept;
}
