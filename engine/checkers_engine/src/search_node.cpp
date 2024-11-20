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

    bool is_game_over_material(const SearchNode& node) {
        unsigned int black_pieces {0};
        unsigned int white_pieces {0};

        for (game::Idx i {0}; i < 32; i++) {
            if (static_cast<unsigned char>(node.board[i]) & 1u << 0) {  // TODO opt.
                black_pieces++;
            } else if (static_cast<unsigned char>(node.board[i]) & 1u << 1) {
                white_pieces++;
            }
        }

        if (black_pieces == 0 || white_pieces == 0) {
            return true;
        }

        return false;
    }
}
