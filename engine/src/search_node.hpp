#pragma once

#include "game.hpp"

namespace search {
    struct SearchNode {
        game::Board board {};
        unsigned int plies {0u};
        unsigned int plies_without_advancement {0u};

        const SearchNode* previous {nullptr};
    };

    void fill_node(SearchNode& current, const SearchNode& previous);
}
