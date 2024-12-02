#include "search_node.hpp"

namespace search {
    void fill_node(SearchNode& current, const SearchNode& previous) {
        current.board = previous.board;
        current.player = previous.player;
        current.plies = previous.plies;
        current.plies_without_advancement = previous.plies_without_advancement;

        current.previous = &previous;
    }

    bool is_forty_move_rule(const SearchNode& node) {
        return node.plies_without_advancement == 80;
    }

    bool is_threefold_repetition_rule(const SearchNode& node) {
        const SearchNode* prev_node {node.previous};

        unsigned int repetitions {1};

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
}
