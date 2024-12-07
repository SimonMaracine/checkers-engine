#pragma once

#include <vector>

#include "game.hpp"
#include "search_node.hpp"
#include "evaluation.hpp"
#include "parameters.hpp"
#include "transposition_table.hpp"
#include "moves.hpp"
#include "array.hpp"

namespace search {
    class Search {
    public:
        Search(transposition_table::TranspositionTable& transposition_table, const parameters::Parameters& parameters);

        game::Move search(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played,
            int max_depth
        );

        bool* get_should_stop() noexcept { return &m_should_stop; }
    private:
        // Return positive if the side to move is doing better and negative if the opposite side is doing better
        evaluation::Eval alpha_beta(  // TODO noexcept
            int depth,
            int plies_root,
            evaluation::Eval alpha,
            evaluation::Eval beta,
            const SearchNode& current_node,
            PvLine& p_line,
            const PvLine& pv_in
        );

        const SearchNode& setup_nodes(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );

        void setup_parameters(const parameters::Parameters& parameters);
        void fill_pv(PvLine& p_line, const PvLine& line, game::Move move) noexcept;
        void reorder_moves_pv(moves::Moves& moves, const PvLine& pv_in, int plies_root) noexcept;
        void reset_after_search_iteration() noexcept;

        bool m_should_stop {false};
        bool m_can_stop {false};
        bool m_reached_left_most_path {false};
        int m_nodes_evaluated {0};
        int m_transpositions {0};

        parameters::SearchParameters m_parameters;

        // The current and previous positions (for threefold repetition)
        // node0, node1, node2, ..., nodeN (current)
        array::Array<SearchNode, 80> m_nodes;

        transposition_table::TranspositionTable& m_transposition_table;
    };
}
