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
    )
        : m_cv(cv), m_lock(lock), m_result_available(result_available) {
        m_parameters.PIECE = parameter_piece;
        m_parameters.DEPTH = parameter_depth;
    }

    std::optional<game::Move> Search::search(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played
    ) {
        SearchNode& current_node {setup_nodes(position, previous_positions, moves_played)};

        const auto start {std::chrono::steady_clock::now()};

        const evaluation::Eval evaluation {minimax(static_cast<unsigned int>(m_parameters.DEPTH), 0, current_node)};

        const auto end {std::chrono::steady_clock::now()};
        const double time {std::chrono::duration<double>(end - start).count()};

        messages::info(m_nodes_evaluated, evaluation, time);

        // If move is invalid, then the game must be over
        // The engine actually waits for a result from the search algorithm, so invalid moves from too little time are impossible
        // Notify the other thread that a "result" is available
        if (game::is_move_invalid(m_best_move)) {
            notify_result_available();
            return std::nullopt;
        }

        return std::make_optional(m_best_move);
    }

    evaluation::Eval Search::minimax(
        unsigned int depth,
        unsigned int plies_from_root,
        const SearchNode& current_node
    ) {
        if (m_should_stop) {
            m_nodes_evaluated++;
            return evaluation::static_evaluation(current_node, m_parameters);
        }

        if (depth == 0 || game::is_game_over(current_node)) {
            m_nodes_evaluated++;
            return evaluation::static_evaluation(current_node, m_parameters);
        }

        if (forty_move_rule(current_node)) {  // Game over
            return 0;
        }

        if (threefold_repetition_rule(current_node)) {  // Game over
            return 0;
        }

        if (current_node.player == game::Player::Black) {
            evaluation::Eval min_evaluation {std::numeric_limits<evaluation::Eval>::max()};

            const auto moves {moves::generate_moves(current_node.board, current_node.player)};

            if (moves.empty()) {  // Game over
                m_nodes_evaluated++;
                return evaluation::static_evaluation(current_node, m_parameters);
            }

            for (const game::Move& move : moves) {
                SearchNode new_node;
                fill_node(new_node, current_node);

                moves::play_move(new_node, move);

                const evaluation::Eval evaluation {
                    minimax(depth - 1, plies_from_root + 1, new_node)
                };

                if (evaluation < min_evaluation) {
                    min_evaluation = evaluation;

                    if (plies_from_root == 0) {
                        m_best_move = move;
                        notify_result_available();
                    }
                }
            }

            return min_evaluation;
        } else {
            evaluation::Eval max_evaluation {std::numeric_limits<evaluation::Eval>::min()};

            const auto moves {moves::generate_moves(current_node.board, current_node.player)};

            if (moves.empty()) {  // Game over
                m_nodes_evaluated++;
                return evaluation::static_evaluation(current_node, m_parameters);
            }

            for (const game::Move& move : moves) {
                SearchNode new_node;
                fill_node(new_node, current_node);

                moves::play_move(new_node, move);

                const evaluation::Eval evaluation {
                    minimax(depth - 1, plies_from_root + 1, new_node)
                };

                if (evaluation > max_evaluation) {
                    max_evaluation = evaluation;

                    if (plies_from_root == 0) {
                        m_best_move = move;
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

        for (std::size_t i {0}; i < previous_positions.size(); i++) {
            const game::Position& position {previous_positions[i]};
            const game::Move& move {moves_played[i]};

            if (is_advancement(position.board, move)) {
                // Clear any previous inserted nodes, as they don't need to be checked
                m_nodes.clear();
            } else {
                m_nodes.push_back(create_node(position.board, position.player, position.plies, position.plies_without_advancement));
            }
        }

        m_nodes.push_back(create_node(position.board, position.player, position.plies, position.plies_without_advancement));

        // Go backwards and link the nodes
        for (std::size_t i {m_nodes.size() - 1}; i > 0; i--) {
            m_nodes[i].previous = &m_nodes[i - 1];
        }

        assert(!m_nodes.empty());

        return m_nodes.back();
    }

    bool Search::is_advancement(const game::Board& board, const game::Move& move) {
        // Must be called before the move is made

        if (move.type == game::MoveType::Normal) {
            return !game::is_king_piece(board[move.normal.source_index]);
        } else {
            return true;
        }
    }

    void Search::notify_result_available() {
        if (!m_notified_result_available) {
            m_result_available = true;  // The lock is still being held
            m_lock.unlock();
            m_cv.notify_one();

            m_notified_result_available = true;
        }
    }
}
