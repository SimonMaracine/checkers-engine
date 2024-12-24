#include "search.hpp"

#include <algorithm>
#include <utility>
#include <iterator>
#include <cstring>
#include <cassert>

#include "messages.hpp"

// https://web.archive.org/web/20071030220820/http://www.brucemo.com/compchess/programming/minmax.htm
// https://web.archive.org/web/20071030084528/http://www.brucemo.com/compchess/programming/alphabeta.htm
// https://web.archive.org/web/20071031100056/http://www.brucemo.com/compchess/programming/iterative.htm
// https://web.archive.org/web/20071031100114/http://www.brucemo.com/compchess/programming/pv.htm
// https://www.chessprogramming.org/Principal_Variation
// https://www.chessprogramming.org/Leftmost_Node
// https://web.archive.org/web/20071027170528/http://www.brucemo.com/compchess/programming/quiescent.htm
// https://web.archive.org/web/20071031095918/http://www.brucemo.com/compchess/programming/aspiration.htm
// https://www.chessprogramming.org/Aspiration_Windows

namespace search {
    Search::Search(
        int search_sequence,
        const parameters::Parameters& parameters,
        transposition_table::TranspositionTable& transposition_table
    ) noexcept
        : m_search_sequence(search_sequence), m_transposition_table(transposition_table) {
        setup_parameters(parameters);
    }

    game::Move Search::search(
        const game::GamePosition& position,
        const std::vector<game::GamePosition>& previous_positions,
        const std::vector<game::Move>& moves_played,
        int max_depth,
        double max_time
    ) noexcept {
        // The TT is not cleared between moves, but only between games
        // The ply of the moves is used to identify old TT entries

        const game::SearchNode& current_node {setup_nodes(position, previous_positions, moves_played)};

        game::PvLine last_pv_line;

        evaluation::Eval alpha {evaluation::WINDOW_MIN};
        evaluation::Eval beta {evaluation::WINDOW_MAX};

        m_max_time = max_time;
        m_begin_search = std::chrono::steady_clock::now();

        for (int depth {1}; depth <= std::min(max_depth, game::MAX_DEPTH);) {
            game::PvLine line;

            const auto begin {std::chrono::steady_clock::now()};

            const evaluation::Eval evaluation {
                alpha_beta(depth, 0, alpha, beta, current_node, line, last_pv_line)
            };

            const auto end {std::chrono::steady_clock::now()};

            check_max_time(end);

            if (m_should_stop & m_can_stop) {
                // Exit immediately; discard the PV, as it's probably broken
                break;
            }

            // If we fell out of the window, then retry the last search with a full window
            if (evaluation <= alpha || evaluation >= beta) {
                alpha = evaluation::WINDOW_MIN;
                beta = evaluation::WINDOW_MAX;
                continue;
            }

            // Setup an appropriate window for the next search
            alpha = evaluation - window_delta();
            beta = evaluation + window_delta();

            last_pv_line = line;

            // This can throw, but if it does, it's game over anyway
            messages::info(
                m_nodes_evaluated,
                m_transpositions,
                depth,
                evaluation * evaluation::perspective(current_node),
                std::chrono::duration<double>(end - begin).count(),
                line.moves,
                line.size
            );

            // If we got no PV, then the game must be over
            if (line.size == 0) {
                return game::NULL_MOVE;
            }

            // After the first iteration, we have some legal move available
            m_can_stop = true;

            reset_after_search_iteration();

            depth++;
        }

        assert(last_pv_line.size > 0);

        return last_pv_line.moves[0];
    }

    evaluation::Eval Search::alpha_beta(
        int depth,
        int plies_root,
        evaluation::Eval alpha,
        evaluation::Eval beta,
        const game::SearchNode& current_node,
        game::PvLine& p_line,
        const game::PvLine& pv
    ) noexcept {
        if (m_nodes_evaluated % 70'000 == 0) {
            // Check the time every now and then
            check_max_time(std::chrono::steady_clock::now());
        }

        if (m_should_stop & m_can_stop) {
            // Discard this search; this should only happen for iterations 2 onwards
            return 0;
        }

        // Generate moves here, because we need to check game over condition on unavailable legal moves
        // At the same time, this also checks game over on unavailable material, 2 in 1
        auto moves {moves::generate_moves(current_node)};

        if (moves.empty()) {  // Game over
            p_line.size = 0;
            m_nodes_evaluated++;
            return evaluation::MIN + plies_root;  // Encourage the winning side to finish earlier (though rare)
        }

        // Then check for tie

        if (game::is_forty_move_rule(current_node)) {  // Game over
            p_line.size = 0;
            m_nodes_evaluated++;
            return 0;
        }

        if (game::is_threefold_repetition_rule(current_node)) {  // Game over
            p_line.size = 0;
            m_nodes_evaluated++;
            return 0;
        }

        // The game is not over
        // If we reached maximum depth and there are no captures available, return heuristic value
        // This is a form of quiescent alpha beta
        if (depth <= 0 && !game::is_move_capture(moves[0])) {
            p_line.size = 0;
            m_nodes_evaluated++;
            return evaluation::static_evaluation(current_node, m_parameters) * evaluation::perspective(current_node);
        }

        // We don't insert game over evaluations into the TT, as it may cause problems

        game::Move hash_move {game::NULL_MOVE};

        // Don't check the TT one ply from the root of the search
        if (plies_root > 1) {
            const auto [evaluation, move] {
                m_transposition_table.load(current_node.key, current_node.signature, depth, alpha, beta)
            };

            if (evaluation != evaluation::UNKNOWN) {
                // TT may greatly disturb the PV, even making it sometimes non sensical
                // Cut out the non-sense from the PV
                p_line.size = 0;

                m_transpositions++;
                return evaluation;
            } else {
                // Retrieve the hash move which is either something or null; it works
                hash_move = move;
            }
        }

        // It's very important to reorder the moves based on the previous PV first
        reorder_moves_pv(moves, pv, plies_root);

        // Reorder based on the hash move second, which may be null, which is fine
        reorder_moves_hash_move(moves, hash_move);

        game::PvLine line;
        auto flag {transposition_table::Flag::Alpha};
        game::Move best_move {game::NULL_MOVE};

        for (const game::Move move : moves) {
            game::SearchNode new_node;
            game::fill_node(new_node, current_node);

            game::play_move(new_node, move);

            const evaluation::Eval evaluation {
                -alpha_beta(depth - 1, plies_root + 1, -beta, -alpha, new_node, line, pv)
            };

            // We need to check for the stop flag here too, because we previously just returned 0,
            // which would have been evaluated and the move put into the TT
            if (m_should_stop & m_can_stop) {
                return 0;
            }

            // Now we stop using the PV from the last iteration
            m_reached_left_most_path = true;

            // Check if the move was way too good for the opponent to let us play it
            // If so, don't evaluate the rest of the moves, because the opponent will not let us get here
            if (evaluation >= beta) {
                m_transposition_table.store(
                    current_node.key,
                    current_node.signature,
                    depth,
                    m_search_sequence,
                    transposition_table::Flag::Beta,
                    beta,
                    move
                );

                return beta;
            }

            // Check if we found a better move than the last time
            if (evaluation > alpha) {
                alpha = evaluation;

                flag = transposition_table::Flag::Exact;
                best_move = move;

                fill_pv(p_line, line, move);
            }
        }

        // Null moves may be inserted into the TT; flags are alpha
        m_transposition_table.store(current_node.key, current_node.signature, depth, m_search_sequence, flag, alpha, best_move);

        return alpha;
    }

    const game::SearchNode& Search::setup_nodes(
        const game::GamePosition& position,
        const std::vector<game::GamePosition>& previous_positions,
        const std::vector<game::Move>& moves_played
    ) noexcept {
        // Mark this noexcept, because it's a logic error for it to throw

        assert(previous_positions.size() == moves_played.size());

        for (std::size_t i {0}; i < previous_positions.size(); i++) {
            const game::GamePosition& previous_position {previous_positions.at(i)};
            const game::Move move_played {moves_played.at(i)};

            if (game::is_move_advancement(previous_position.board, move_played)) {
                // Clear any previous inserted nodes, as they don't need to be checked
                m_nodes.clear();
            } else {
                m_nodes.push_back({
                    previous_position.board,
                    previous_position.player,
                    previous_position.plies_without_advancement,
                    previous_position.key,
                    previous_position.signature,
                    nullptr
                });
            }
        }

        m_nodes.push_back({
            position.board,
            position.player,
            position.plies_without_advancement,
            position.key,
            position.signature,
            nullptr
        });

        // Go backwards and link the nodes
        for (int i {m_nodes.size() - 1}; i > 0; i--) {
            m_nodes[i].previous = &m_nodes[i - 1];
        }

        assert(!m_nodes.empty());

        return m_nodes.back();
    }

    void Search::setup_parameters(const parameters::Parameters& parameters) noexcept {
        // Mark this noexcept, because it's a logic error for it to throw

        m_parameters.material_pawn = std::get<0>(parameters.at("material_pawn"));
        m_parameters.material_king = std::get<0>(parameters.at("material_king"));
        m_parameters.positioning_pawn = std::get<0>(parameters.at("positioning_pawn"));
        m_parameters.positioning_king = std::get<0>(parameters.at("positioning_king"));
        m_parameters.crowdness = std::get<0>(parameters.at("crowdness"));
    }

    void Search::fill_pv(game::PvLine& p_line, const game::PvLine& line, game::Move move) noexcept {
        p_line.moves[0] = move;
        std::memcpy(p_line.moves + 1, line.moves, line.size * sizeof(move));
        p_line.size = line.size + 1;
    }

    void Search::reorder_moves_pv(moves::Moves& moves, const game::PvLine& pv, int plies_root) const noexcept {
        if ((plies_root >= pv.size) | m_reached_left_most_path) {
            return;
        }

        const auto iter {std::find(moves.begin(), moves.end(), pv.moves[plies_root])};

        assert(iter != moves.end());

        // Place the PV move first
        std::iter_swap(moves.begin(), iter);
    }

    void Search::reorder_moves_hash_move(moves::Moves& moves, game::Move hash_move) noexcept {
        if (hash_move == game::NULL_MOVE) {
            return;
        }

        const auto iter {std::find(moves.begin(), moves.end(), hash_move)};

        assert(iter != moves.end());

        // If the hash move is already the PV move, don't touch it (or there is only one move in the array)
        if (iter == moves.begin()) {
            return;
        }

        // Place the hash move second (first is PV move)
        std::iter_swap(std::next(moves.begin()), iter);
    }

    void Search::reset_after_search_iteration() noexcept {
        m_reached_left_most_path = false;
        m_nodes_evaluated = 0;
        m_transpositions = 0;
    }

    void Search::check_max_time(TimePoint time_point) noexcept {
        if (std::chrono::duration<double>(time_point - m_begin_search).count() > m_max_time) {
            m_should_stop = true;
        }
    }

    evaluation::Eval Search::window_delta() const noexcept {
        return (m_parameters.material_pawn * 80) / 100;
    }
}
