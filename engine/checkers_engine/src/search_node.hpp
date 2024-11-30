#pragma once

#include <cstddef>

#include "game.hpp"

namespace search {
    struct PvLine {
        game::Move moves[100] {};
        std::size_t size {};
    };

    struct SearchNode {
        game::Board board {};
        game::Player player {};  // Next player to move
        unsigned int plies {0};  // TODO might not be needed
        unsigned int plies_without_advancement {0};

        const SearchNode* previous {nullptr};
    };

    void fill_node(SearchNode& current, const SearchNode& previous);
    bool is_forty_move_rule(const SearchNode& node);
    bool is_threefold_repetition_rule(const SearchNode& node);
    bool is_game_over_material(const SearchNode& node);
}
