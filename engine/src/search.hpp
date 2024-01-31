#pragma once

#include <vector>

#include "game.hpp"

namespace search {
    class Search {
    public:
        // TODO initialize parameters in constructor

        game::Move search(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );
    private:
        game::Eval minimax(
            game::Player player,
            unsigned int depth,
            unsigned int plies_from_root
        );

        game::Move best_move {};
        unsigned int nodes_evaluated {};

        struct {
            game::Board board {};
            unsigned int plies {0u};
            unsigned int plies_without_advancement {0u};
        } ctx;

        struct {
            // TODO
        } parameters;
    };
}
