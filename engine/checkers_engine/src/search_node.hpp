#pragma once

#include <cstddef>

#include "game.hpp"
#include "evaluation.hpp"

namespace search {
    struct PvLine {
        game::Move moves[100];
        int size {};
    };

    struct SearchNode {
        game::Board board {};
        game::Player player {};  // Next player to move
        int plies_without_advancement {0};

        const SearchNode* previous {nullptr};
    };

    void fill_node(SearchNode& current, const SearchNode& previous);
    bool is_forty_move_rule(const SearchNode& node);
    bool is_threefold_repetition_rule(const SearchNode& node);

    constexpr evaluation::Eval perspective(const SearchNode& node) {
        return node.player == game::Player::Black ? -1 : 1;
    }
}
