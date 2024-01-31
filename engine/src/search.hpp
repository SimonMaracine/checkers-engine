#pragma once

#include <vector>

#include "game.hpp"
#include "search_node.hpp"
#include "evaluation.hpp"

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
        evaluation::Eval minimax(
            game::Player player,
            unsigned int depth,
            unsigned int plies_from_root,
            SearchNode& current_node
        );

        SearchNode& setup_nodes(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );

        game::Move best_move {};
        unsigned int nodes_evaluated {};

        // The current position and previous positions (for threefold reptition)
        // position0, position1, position2, ..., positionN (current)
        std::vector<SearchNode> nodes;

        struct {
            // TODO
        } parameters;
    };
}
