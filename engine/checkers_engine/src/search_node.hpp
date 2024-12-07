#pragma once

#include <cstddef>

#include "game.hpp"
#include "evaluation.hpp"

namespace search {
    inline constexpr int MAX_DEPTH {90};

    struct PvLine {
        game::Move moves[MAX_DEPTH];
        int size {};
    };

    struct SearchNode {
        game::Board board {};
        game::Player player {};  // Next player to move
        int plies_without_advancement {0};

        const SearchNode* previous {nullptr};
    };

    constexpr void fill_node(SearchNode& current, const SearchNode& previous) noexcept {
        current.board = previous.board;
        current.player = previous.player;
        current.plies_without_advancement = previous.plies_without_advancement;

        current.previous = &previous;
    }

    constexpr bool is_forty_move_rule(const SearchNode& node) noexcept {
        return node.plies_without_advancement == 80;
    }

    constexpr bool is_threefold_repetition_rule(const SearchNode& node) noexcept {
        const SearchNode* prev_node {node.previous};

        int repetitions {1};

        while (prev_node != nullptr) {
            if (prev_node->board == node.board && prev_node->player == node.player) {
                if (++repetitions == 3) {
                    return true;
                }
            }

            prev_node = prev_node->previous;
        }

        return false;
    }

    constexpr evaluation::Eval perspective(const SearchNode& node) noexcept {
        return node.player == game::Player::Black ? -1 : 1;
    }
}
