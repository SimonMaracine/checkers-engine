#pragma once

#include <vector>

#include "game.hpp"
#include "search_node.hpp"

namespace moves {
    void play_move(game::Position& position, const game::Move& move);
    void play_move(search::SearchNode& node, const game::Move& move);
    std::vector<game::Move> generate_moves(const game::Board& board, game::Player player);
}
