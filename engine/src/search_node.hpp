#pragma once

#include "game.hpp"

namespace search {
    struct SearchNode {
        game::Board board {};
        game::Player player {};
        unsigned int plies {0u};
        unsigned int plies_without_advancement {0u};

        const SearchNode* previous {nullptr};
    };

    void fill_node(SearchNode& current, const SearchNode& previous);
    bool eighty_move_rule(const SearchNode& node);
    bool threefold_repetition_rule(const SearchNode& node);
}
