#include "search.hpp"

#include <limits>
#include <chrono>
#include <cassert>
#include <cstddef>

#include "moves.hpp"
#include "messages.hpp"

namespace search {
    Search::Search(
        std::condition_variable& cv,
        std::unique_lock<std::mutex>& lock,
        bool& result_available,
        int parameter_piece,
        int parameter_depth
    ) : cv(cv), lock(lock), result_available(result_available) {
        parameters.PIECE = parameter_piece;
        parameters.DEPTH = parameter_depth;
    }

    game::Move Search::search(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played
    ) {
        SearchNode& current_node {setup_nodes(position, previous_positions, moves_played)};

        const auto start {std::chrono::steady_clock::now()};

        const evaluation::Eval evaluation {minimax(static_cast<unsigned int>(parameters.DEPTH), 0u, current_node)};

        const auto end {std::chrono::steady_clock::now()};
        const double time {std::chrono::duration<double>(end - start).count()};

        messages::info(nodes_evaluated, evaluation, time);

        return best_move;
    }

    evaluation::Eval Search::minimax(
        unsigned int depth,
        unsigned int plies_from_root,
        SearchNode& current_node
    ) {
        if (should_stop) {
            return 0;
        }

        if (depth == 0u || game::is_game_over(current_node)) {
            nodes_evaluated++;
            return evaluation::static_evaluation(current_node, parameters);
        }

        if (forty_move_rule(current_node)) {
            nodes_evaluated++;
            return 0;
        }

        if (threefold_repetition_rule(current_node)) {
            nodes_evaluated++;
            return 0;
        }

        if (current_node.player == game::Player::Black) {
            evaluation::Eval min_evaluation {std::numeric_limits<evaluation::Eval>::max()};

            const auto moves {moves::generate_moves(current_node.board, current_node.player)};

            if (moves.empty()) {  // Game over
                nodes_evaluated++;
                return evaluation::static_evaluation(current_node, parameters);
            }

            for (const game::Move& move : moves) {
                SearchNode new_node;
                fill_node(new_node, current_node);

                moves::play_move(new_node, move);

                const evaluation::Eval evaluation {
                    minimax(depth - 1u, plies_from_root + 1u, new_node)
                };

                if (evaluation < min_evaluation) {
                    min_evaluation = evaluation;

                    if (plies_from_root == 0u) {
                        best_move = move;
                        notify_result_available();
                    }
                }
            }

            return min_evaluation;
        } else {
            evaluation::Eval max_evaluation {std::numeric_limits<evaluation::Eval>::min()};

            const auto moves {moves::generate_moves(current_node.board, current_node.player)};

            if (moves.empty()) {  // Game over
                nodes_evaluated++;
                return evaluation::static_evaluation(current_node, parameters);
            }

            for (const game::Move& move : moves) {
                SearchNode new_node;
                fill_node(new_node, current_node);

                moves::play_move(new_node, move);

                const evaluation::Eval evaluation {
                    minimax(depth - 1u, plies_from_root + 1u, new_node)
                };

                if (evaluation > max_evaluation) {
                    max_evaluation = evaluation;

                    if (plies_from_root == 0u) {
                        best_move = move;
                        notify_result_available();
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

        for (std::size_t i {0u}; i < previous_positions.size(); i++) {
            const game::Position& position {previous_positions[i]};
            const game::Move& move {moves_played[i]};

            if (is_advancement(position, move)) {
                nodes.clear();
            } else {
                SearchNode node;
                node.board = position.board;
                node.player = position.player;
                node.plies = position.plies;
                node.plies_without_advancement = position.plies_without_advancement;

                nodes.push_back(node);
            }
        }

        SearchNode current_node;
        current_node.board = position.board;
        current_node.player = position.player;
        current_node.plies = position.plies;
        current_node.plies_without_advancement = position.plies_without_advancement;

        nodes.push_back(current_node);

        // Go backwards and link the nodes
        for (std::size_t i {nodes.size() - 1u}; i > 0u; i--) {
            nodes[i].previous = &nodes[i - 1u];
        }

        assert(!nodes.empty());

        return nodes.back();
    }

    bool Search::is_advancement(const game::Position& position, const game::Move& move) {
        if (move.type == game::MoveType::Normal) {
            return !game::is_king_piece(position.board[move.normal.destination_index]);
        } else {
            return true;
        }
    }

    void Search::notify_result_available() {
        if (!notified_result_available) {
            result_available = true;  // The lock is still being held
            lock.unlock();
            cv.notify_one();

            notified_result_available = true;
        }
    }
}
