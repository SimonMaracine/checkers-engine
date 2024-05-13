#include "search_node.hpp"

namespace search {
    void fill_node(SearchNode& current, const SearchNode& previous) {
        current.board = previous.board;
        current.player = previous.player;
        current.plies = previous.plies;
        current.plies_without_advancement = previous.plies_without_advancement;

        current.previous = &previous;
    }

    bool eighty_move_rule(const SearchNode& node) {
        return node.plies_without_advancement == 80u;
    }

    bool threefold_repetition_rule(const SearchNode& node) {
        const SearchNode* prev {node.previous};

        unsigned int repetitions {1u};

        while (prev != nullptr) {
            if (prev->board == node.board && prev->player == node.player) {
                if (++repetitions == 3u) {
                    return true;
                }
            }

            prev = prev->previous;
        }

        return false;
    }
}
