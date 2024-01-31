#pragma once

#include <vector>

#include "game.hpp"

namespace moves {
    void play_move(game::Position& position, const game::Move& move);
    void play_move(game::Board& board, game::Player& player, unsigned int& plies, unsigned int& plies_without_advancement, const game::Move& move);
    void unplay_move(game::Board& board, game::Player& player, unsigned int& plies, unsigned int& plies_without_advancement, const game::Move& move);
    std::vector<game::Move> generate_moves(const game::Board& board, game::Player player);
}
