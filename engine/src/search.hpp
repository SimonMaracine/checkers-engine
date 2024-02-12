#pragma once

#include <vector>

#include "game.hpp"
#include "search_node.hpp"
#include "evaluation.hpp"

namespace search {
    class Search {
    public:
        Search(int parameter_piece);
        ~Search() = default;

        Search(const Search&) = delete;
        Search& operator=(const Search&) = delete;
        Search(Search&&) = delete;
        Search& operator=(Search&&) = delete;

        game::Move search(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );
    private:
        evaluation::Eval minimax(
            unsigned int depth,
            unsigned int plies_from_root,
            SearchNode& current_node
        );

        SearchNode& setup_nodes(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );

        bool is_advancement(const game::Position& position, const game::Move& move);

        game::Move best_move {};
        unsigned int nodes_evaluated {};

        // The current position and previous positions (for threefold reptition)
        // position0, position1, position2, ..., positionN (current)
        std::vector<SearchNode> nodes;

        evaluation::Parameters parameters;
    };
}
