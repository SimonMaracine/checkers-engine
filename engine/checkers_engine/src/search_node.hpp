#pragma once

#include "game.hpp"

namespace search {
    struct SearchNode {
        game::Board board {};
        game::Player player {};
        unsigned int plies {0};  // TODO might not be needed
        unsigned int plies_without_advancement {0};

        const SearchNode* previous {nullptr};
    };

    SearchNode create_node(const game::Board& board, game::Player player, unsigned int plies, unsigned int plies_without_advancement);
    void fill_node(SearchNode& current, const SearchNode& previous);
    bool forty_move_rule(const SearchNode& node);
    bool threefold_repetition_rule(const SearchNode& node);
}
