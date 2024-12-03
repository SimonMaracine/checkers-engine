#include "search.hpp"

#include <chrono>
#include <algorithm>
#include <utility>
#include <cstddef>
#include <cstring>
#include <cassert>

#include "moves.hpp"
#include "messages.hpp"

// https://web.archive.org/web/20071030220820/http://www.brucemo.com/compchess/programming/minmax.htm
// https://web.archive.org/web/20071030084528/http://www.brucemo.com/compchess/programming/alphabeta.htm
// https://web.archive.org/web/20071031100056/http://www.brucemo.com/compchess/programming/iterative.htm
// https://web.archive.org/web/20071031100114/http://www.brucemo.com/compchess/programming/pv.htm
// https://www.chessprogramming.org/Principal_Variation
// https://www.chessprogramming.org/Leftmost_Node

namespace search {
    Search::Search(
        std::condition_variable& cv,
        std::unique_lock<std::mutex>& lock,
        bool& best_move_available,
        transposition_table::TranspositionTable& transposition_table,
        const parameters::Parameters& parameters
    )
        : m_cv(cv), m_lock(lock), m_best_move_available(best_move_available), m_transposition_table(transposition_table) {
        setup_parameters(parameters);
    }

    std::optional<game::Move> Search::search(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played,
        unsigned int max_depth
    ) {
        const SearchNode& current_node {setup_nodes(position, previous_positions, moves_played)};

        PvLine last_pv_line;

        for (unsigned int i {1}; i <= max_depth; i++) {
            const auto begin {std::chrono::steady_clock::now()};

            PvLine line;

            const evaluation::Eval evaluation {
                alpha_beta(i, 0, evaluation::WINDOW_MIN, evaluation::WINDOW_MAX, current_node, line, last_pv_line)
            };

            if (m_should_stop) {
                // Exit immediately; discard the PV, as it's probably broken
                break;
            }

            last_pv_line = line;

            const auto end {std::chrono::steady_clock::now()};
            const double time {std::chrono::duration<double>(end - begin).count()};

            messages::info(
                m_nodes_evaluated,
                m_transpositions,
                i,
                evaluation * perspective(current_node),
                time,
                line.moves,
                line.size
            );

            // If we got no PV, then the game must be over
            // The engine actually waits for a result from the search algorithm, so invalid PV from too little time is impossible
            // Notify the main thread that a "result" is available
            if (last_pv_line.size == 0) {
                notify_result_available();
                return std::nullopt;
            }

            reset_after_search_iteration();

            // After the first iteration, we have some legal move available
            notify_result_available();
        }

        return std::make_optional(last_pv_line.moves[0]);
    }

    evaluation::Eval Search::alpha_beta(
        unsigned int depth,
        unsigned int plies_root,
        evaluation::Eval alpha,
        evaluation::Eval beta,
        const SearchNode& current_node,
        PvLine& p_line,
        const PvLine& pv_in
    ) {
        if (m_should_stop) {
            // Discard this search; this should only happen for iterations 2 onwards
            return 0;
        }

        // Generate moves here, because we need to check game over condition on unavailable legal moves
        // At the same time, this also checks game over on unavailable material, 2 in 1
        auto moves {moves::generate_moves(current_node.board, current_node.player)};

        if (moves.empty()) {  // Game over
            p_line.size = 0;
            m_nodes_evaluated++;
            return (current_node.player == game::Player::Black ? evaluation::MAX : evaluation::MIN) * perspective(current_node);
        }

        // Then check for tie

        if (is_forty_move_rule(current_node)) {  // Game over
            p_line.size = 0;
            m_nodes_evaluated++;
            return 0;
        }

        if (is_threefold_repetition_rule(current_node)) {  // Game over
            p_line.size = 0;
            m_nodes_evaluated++;
            return 0;
        }

        // The game is not over
        // If we reached maximum depth, return heuristic value
        if (depth == 0) {
            p_line.size = 0;
            m_nodes_evaluated++;
            return evaluation::static_evaluation(current_node, m_parameters) * search::perspective(current_node);
        }

        // Don't check the TT at the root of the search
        if (plies_root > 0) {
            const auto [evaluation, move] {
                m_transposition_table.retrieve({ current_node.board, current_node.player }, depth, alpha, beta)
            };

            if (evaluation != evaluation::INVALID) {
                // TT may greatly disturb the PV, even making it sometimes non sensical
                // Cut out the non-sense from the PV
                p_line.size = 0;

                m_transpositions++;
                return evaluation;
            }
        }

        // It's very important to reorder the moves based on the previous PV
        reorder_moves_pv(moves, pv_in, plies_root);

        PvLine line;
        auto node_type {transposition_table::NodeType::All};
        game::Move best_move {};

        for (const game::Move& move : moves) {
            SearchNode new_node;
            fill_node(new_node, current_node);

            game::play_move(new_node, move);

            const evaluation::Eval evaluation {
                -alpha_beta(depth - 1, plies_root + 1, -beta, -alpha, new_node, line, pv_in)
            };

            // We need to check for the stop flag here too, because we previously just returned 0,
            // which would have been evaluated and the move put into the TT
            // This then would have caused next iterations to retrive bad moves from the TT
            if (m_should_stop) {
                return 0;
            }

            // Now we stop using the PV from the last iteration
            m_reached_left_most_path = true;

            // Check if the move was way too good for the opponent to let us play it
            // If so, don't evaluate the rest of the moves, because the opponent will not let us get here
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

            // Check if we found a better move than the last time
            if (evaluation > alpha) {
                alpha = evaluation;

                node_type = transposition_table::NodeType::Pv;
                best_move = move;

                fill_pv(p_line, line, move);
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

    void Search::setup_parameters(const parameters::Parameters& parameters) {
        m_parameters.material_pawn = std::get<0>(parameters.at("material_pawn"));
        m_parameters.material_king = std::get<0>(parameters.at("material_king"));
        m_parameters.positioning_pawn = std::get<0>(parameters.at("positioning_pawn"));
        m_parameters.positioning_king = std::get<0>(parameters.at("positioning_king"));
        m_parameters.crowdness = std::get<0>(parameters.at("crowdness"));
    }

    void Search::fill_pv(PvLine& p_line, const PvLine& line, const game::Move& move) {
        p_line.moves[0] = move;
        std::memcpy(p_line.moves + 1, line.moves, line.size * sizeof(move));
        p_line.size = line.size + 1;
    }

    void Search::reorder_moves_pv(std::vector<game::Move>& moves, const PvLine& pv_in, unsigned int plies_root) {
        if (plies_root >= pv_in.size || m_reached_left_most_path) {
            return;
        }

        const auto iter {std::find(moves.begin(), moves.end(), pv_in.moves[plies_root])};

        if (iter == moves.end()) {
            std::abort();
        }

        std::iter_swap(moves.begin(), iter);
    }

    void Search::reset_after_search_iteration() {
        m_reached_left_most_path = false;
        m_nodes_evaluated = 0;
        m_transpositions = 0;
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
