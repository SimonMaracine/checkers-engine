#include "search_node.hpp"

namespace search {
    SearchNode create_node(const game::Board& board, game::Player player, unsigned int plies, unsigned int plies_without_advancement) {
        return {
            board,
            player,
            plies,
            plies_without_advancement
        };
    }

    void fill_node(SearchNode& current, const SearchNode& previous) {
        current.board = previous.board;
        current.player = previous.player;
        current.plies = previous.plies;
        current.plies_without_advancement = previous.plies_without_advancement;

        current.previous = &previous;
    }

    bool forty_move_rule(const SearchNode& node) {
        return node.plies_without_advancement == 80;
    }

    bool threefold_repetition_rule(const SearchNode& node) {
        const SearchNode* prev {node.previous};

        unsigned int repetitions {1};

        while (prev != nullptr) {
            if (prev->board == node.board && prev->player == node.player) {
                if (++repetitions == 3) {
                    return true;
                }
            }

            prev = prev->previous;
        }

        return false;
    }
}
