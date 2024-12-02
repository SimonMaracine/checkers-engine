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

    evaluation::Eval is_game_over_material(const SearchNode& node) {
        int black_pieces {0};
        int white_pieces {0};

        for (int i {0}; i < 32; i++) {
            switch (node.board[i]) {
                case game::Square::None:
                    break;
                case game::Square::Black:
                case game::Square::BlackKing:
                    black_pieces++;
                    break;
                case game::Square::White:
                case game::Square::WhiteKing:
                    white_pieces++;
                    break;
            }
        }

        if (black_pieces == 0) {
            return evaluation::MAX;
        }

        if (white_pieces == 0) {
            return evaluation::MIN;
        }

        return 0;
    }
}
