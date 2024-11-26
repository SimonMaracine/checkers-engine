#include "search.hpp"

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
        transposition_table::TranspositionTable& transposition_table,
        const parameters::Parameters& parameters
    )
        : m_cv(cv), m_lock(lock), m_best_move_available(best_move_available), m_transposition_table(transposition_table) {
        m_parameters.piece = std::get<0>(parameters.at("piece"));
    }

    std::optional<game::Move> Search::search(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played,
        unsigned int depth
    ) {
        const SearchNode& current_node {setup_nodes(position, previous_positions, moves_played)};

        const auto begin {std::chrono::steady_clock::now()};

        const evaluation::Eval evaluation {
            alpha_beta(depth, 0, evaluation::MIN, evaluation::MAX, current_node)
        };

        const auto end {std::chrono::steady_clock::now()};
        const double time {std::chrono::duration<double>(end - begin).count()};

        messages::info(m_nodes_evaluated, evaluation * (position.player == game::Player::Black ? -1 : 1), time);

        // If move is invalid, then the game must be over
        // The engine actually waits for a result from the search algorithm, so invalid moves from too little time are impossible
        // Notify the main thread that a "result" is available
        if (game::is_move_invalid(m_best_move)) {
            notify_result_available();
            return std::nullopt;
        }

        return std::make_optional(m_best_move);
    }

    evaluation::Eval Search::alpha_beta(
        unsigned int depth,
        unsigned int plies_root,
        evaluation::Eval alpha,
        evaluation::Eval beta,
        const SearchNode& current_node
    ) {
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

        const auto [evaluation, move] {
            m_transposition_table.retrieve({ current_node.board, current_node.player }, depth, alpha, beta)
        };

        if (evaluation != evaluation::INVALID) {
            if (plies_root == 0) {
                m_best_move = move;
                notify_result_available();
            }

            return evaluation;
        }

        const auto moves {moves::generate_moves(current_node.board, current_node.player)};

        if (moves.empty()) {  // Game over
            m_nodes_evaluated++;
            return evaluation::static_evaluation(current_node, m_parameters);
        }

        auto node_type {transposition_table::NodeType::All};
        game::Move best_move {};

        for (const game::Move& move : moves) {
            SearchNode new_node;
            fill_node(new_node, current_node);

            moves::play_move(new_node, move);

            const evaluation::Eval evaluation {-alpha_beta(depth - 1, plies_root + 1, -beta, -alpha, new_node)};

            if (evaluation >= beta) {
                m_transposition_table.store(
                    { current_node.board, current_node.player },
                    depth,
                    transposition_table::NodeType::Cut,
                    beta,
                    move
                );

                return beta;
            }

            if (evaluation > alpha) {
                if (plies_root == 0) {
                    m_best_move = move;
                    notify_result_available();
                }

                best_move = move;

                node_type = transposition_table::NodeType::Pv;
                alpha = evaluation;
            }
        }

        m_transposition_table.store({ current_node.board, current_node.player }, depth, node_type, alpha, best_move);

        return alpha;
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
