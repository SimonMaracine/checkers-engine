#include "search.hpp"

#include <limits>
#include <chrono>
#include <cstddef>
#include <cassert>

#include "moves.hpp"
#include "messages.hpp"

namespace search {
    Search::Search(
        std::condition_variable& cv,
        std::unique_lock<std::mutex>& lock,
        bool& best_move_available,
        const parameters::Parameters& parameters
    )
        : m_cv(cv), m_lock(lock), m_best_move_available(best_move_available) {
        m_parameters.piece = std::get<0>(parameters.at("piece"));
    }

    std::optional<game::Move> Search::search(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played,
        unsigned int depth
    ) {
        const SearchNode& current_node {setup_nodes(position, previous_positions, moves_played)};

        const auto start {std::chrono::steady_clock::now()};

        const evaluation::Eval evaluation {minimax(depth, 0, current_node)};

        const auto end {std::chrono::steady_clock::now()};
        const double time {std::chrono::duration<double>(end - start).count()};

        messages::info(m_nodes_evaluated, evaluation, time);

        // If move is invalid, then the game must be over
        // The engine actually waits for a result from the search algorithm, so invalid moves from too little time are impossible
        // Notify the main thread that a "result" is available
        if (game::is_move_invalid(m_best_move)) {
            notify_result_available();
            return std::nullopt;
        }

        return std::make_optional(m_best_move);
    }

    evaluation::Eval Search::minimax(unsigned int depth, unsigned int plies_root, const SearchNode& current_node) {
        if (m_should_stop) {
            m_nodes_evaluated++;
            return evaluation::static_evaluation(current_node, m_parameters);
        }

        if (depth == 0 || is_game_over_material(current_node)) {  // Game over
            m_nodes_evaluated++;
            return evaluation::static_evaluation(current_node, m_parameters);
        }

        if (is_forty_move_rule(current_node)) {  // Game over
            m_nodes_evaluated++;
            return 0;
        }

        if (is_threefold_repetition_rule(current_node)) {  // Game over
            m_nodes_evaluated++;
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
                    minimax(depth - 1, plies_root + 1, new_node)
                };

                if (evaluation < min_evaluation) {
                    min_evaluation = evaluation;

                    if (plies_root == 0) {
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
                    minimax(depth - 1, plies_root + 1, new_node)
                };

                if (evaluation > max_evaluation) {
                    max_evaluation = evaluation;

                    if (plies_root == 0) {
                        m_best_move = move;
                        notify_result_available();
                    }
                }
            }

            return max_evaluation;
        }
    }

    const SearchNode& Search::setup_nodes(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played
    ) {
        assert(previous_positions.size() == moves_played.size());

        for (std::size_t i {0}; i < previous_positions.size(); i++) {
            const game::Position& previous_position {previous_positions.at(i)};
            const game::Move& move_played {moves_played.at(i)};

            if (game::is_move_advancement(previous_position.board, move_played)) {
                // Clear any previous inserted nodes, as they don't need to be checked
                m_nodes.clear();
            } else {
                m_nodes.push_back({
                    previous_position.board,
                    previous_position.player,
                    previous_position.plies,
                    previous_position.plies_without_advancement,
                    nullptr
                });
            }
        }

        m_nodes.push_back({position.board, position.player, position.plies, position.plies_without_advancement, nullptr});

        // Go backwards and link the nodes
        for (std::size_t i {m_nodes.size() - 1}; i > 0; i--) {
            m_nodes.at(i).previous = &m_nodes.at(i - 1);
        }

        assert(!m_nodes.empty());

        return m_nodes.back();
    }

    void Search::notify_result_available() {
        if (!m_notified_result_available) {
            m_best_move_available = true;  // The lock is still being held
            m_lock.unlock();
            m_cv.notify_one();

            m_notified_result_available = true;
        }
    }
}
