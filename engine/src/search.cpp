#include "search.hpp"

#include <limits>
#include <chrono>
#include <cassert>

#include "moves.hpp"

namespace search {
    Search::Search(int parameter_piece) {
        parameters.PIECE = parameter_piece;
    }

    game::Move Search::search(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played
    ) {
        SearchNode& current_node {setup_nodes(position, previous_positions, moves_played)};

        const auto start {std::chrono::steady_clock::now()};

        const evaluation::Eval evaluation {minimax(position.position.player, 4u, 0u, current_node)};

        const auto end {std::chrono::steady_clock::now()};

        // TODO
        const double time {std::chrono::duration<double>(end - start).count()};

        return best_move;
    }

    evaluation::Eval Search::minimax(
        game::Player player,
        unsigned int depth,
        unsigned int plies_from_root,
        SearchNode& current_node
    ) {
        if (depth == 0u || game::is_game_over(current_node)) {
            nodes_evaluated++;
            return evaluation::static_evaluation(current_node, parameters);
        }

        // TODO threefold repetition, eighty move rule

        if (player == game::Player::Black) {
            evaluation::Eval min_evaluation {std::numeric_limits<evaluation::Eval>::max()};

            const auto moves {moves::generate_moves(current_node.board, player)};

            if (moves.empty()) {
                nodes_evaluated++;
                return evaluation::static_evaluation(current_node, parameters);
            }

            for (const game::Move& move : moves) {
                SearchNode new_node;
                fill_node(new_node, current_node);

                moves::play_move(new_node, move);

                const evaluation::Eval evaluation {
                    minimax(game::Player::White, depth - 1u, plies_from_root + 1u, new_node)
                };

                if (evaluation < min_evaluation) {
                    min_evaluation = evaluation;

                    if (plies_from_root == 0u) {
                        best_move = move;
                    }
                }
            }

            return min_evaluation;
        } else {
            evaluation::Eval max_evaluation {std::numeric_limits<evaluation::Eval>::min()};

            const auto moves {moves::generate_moves(current_node.board, player)};

            if (moves.empty()) {
                nodes_evaluated++;
                return evaluation::static_evaluation(current_node, parameters);
            }

            for (const game::Move& move : moves) {
                SearchNode new_node;
                fill_node(new_node, current_node);

                moves::play_move(new_node, move);

                const evaluation::Eval evaluation {
                    minimax(game::Player::Black, depth - 1u, plies_from_root + 1u, new_node)
                };

                if (evaluation > max_evaluation) {
                    max_evaluation = evaluation;

                    if (plies_from_root == 0u) {
                        best_move = move;
                    }
                }
            }

            return max_evaluation;
        }
    }

    SearchNode& Search::setup_nodes(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played
    ) {
        assert(previous_positions.size() == moves_played.size());

        // TODO insert previous positions into nodes

        SearchNode current_node;
        current_node.board = position.position.board;
        current_node.plies = position.plies;
        current_node.plies_without_advancement = position.plies_without_advancement;

        nodes.push_back(current_node);

        assert(!nodes.empty());

        return current_node;
    }
}
