#pragma once

#include <vector>
#include <chrono>

#include "game.hpp"
#include "evaluation.hpp"
#include "parameters.hpp"
#include "transposition_table.hpp"
#include "moves.hpp"
#include "array.hpp"

namespace search {
    class Search {
    public:
        Search(
            int search_sequence,
            const parameters::Parameters& parameters,
            transposition_table::TranspositionTable& transposition_table
        ) noexcept;

        game::Move search(
            const game::GamePosition& position,
            const std::vector<game::GamePosition>& previous_positions,
            const std::vector<game::Move>& moves_played,
            int max_depth,
            double max_time
        ) noexcept;

        bool* get_should_stop() noexcept { return &m_should_stop; }
    private:
        using TimePoint = std::chrono::steady_clock::time_point;

        // Return positive if the side to move is doing better and negative if the opposite side is doing better
        evaluation::Eval alpha_beta(
            int depth,
            int plies_root,
            evaluation::Eval alpha,
            evaluation::Eval beta,
            const game::SearchNode& current_node,
            game::PvLine& p_line,
            const game::PvLine& pv
        ) noexcept;

        const game::SearchNode& setup_nodes(
            const game::GamePosition& position,
            const std::vector<game::GamePosition>& previous_positions,
            const std::vector<game::Move>& moves_played
        ) noexcept;

        void setup_parameters(const parameters::Parameters& parameters) noexcept;
        static void fill_pv(game::PvLine& p_line, const game::PvLine& line, game::Move move) noexcept;
        void reorder_moves_pv(moves::Moves& moves, const game::PvLine& pv, int plies_root) const noexcept;
        static void reorder_moves_hash_move(moves::Moves& moves, game::Move hash_move) noexcept;
        void reset_after_search_iteration() noexcept;
        void check_max_time(TimePoint time_point) noexcept;
        evaluation::Eval window_delta() const noexcept;

        bool m_should_stop {false};
        bool m_can_stop {false};
        bool m_reached_left_most_path {false};
        int m_nodes_evaluated {0};
        int m_transpositions {0};

        // The number of this particular search
        int m_search_sequence {};

        parameters::SearchParameters m_parameters;

        // Used to check for max time
        double m_max_time {};
        TimePoint m_begin_search {};

        // The current and previous positions (for threefold repetition)
        // node0, node1, node2, ..., nodeN (current)
        array::Array<game::SearchNode, 81> m_nodes;

        transposition_table::TranspositionTable& m_transposition_table;
    };
}
