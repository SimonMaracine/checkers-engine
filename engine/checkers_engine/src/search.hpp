#pragma once

#include <vector>
#include <condition_variable>
#include <mutex>
#include <optional>

#include "game.hpp"
#include "search_node.hpp"
#include "evaluation.hpp"
#include "parameters.hpp"
#include "transposition_table.hpp"

namespace search {
    class Search {
    public:
        Search(
            std::condition_variable& cv,
            std::unique_lock<std::mutex>& lock,
            bool& best_move_available,
            transposition_table::TranspositionTable& transposition_table,
            const parameters::Parameters& parameters
        );

        ~Search() = default;

        Search(const Search&) = delete;
        Search& operator=(const Search&) = delete;
        Search(Search&&) = delete;
        Search& operator=(Search&&) = delete;

        std::optional<game::Move> search(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played,
            int max_depth
        );

        bool* get_should_stop() { return &m_should_stop; }
    private:
        // Return positive if the side to move is doing better and negative if the opposite side is doing better
        evaluation::Eval alpha_beta(
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
        void fill_pv(PvLine& p_line, const PvLine& line, const game::Move& move);
        void reorder_moves_pv(std::vector<game::Move>& moves, const PvLine& pv_in, int plies_root);
        void reset_after_search_iteration();
        void notify_result_available();

        bool m_notified_result_available {false};
        bool m_should_stop {false};
        bool m_reached_left_most_path {false};
        int m_nodes_evaluated {};
        int m_transpositions {};

        // The current and previous positions (for threefold repetition)
        // node0, node1, node2, ..., nodeN (current)
        std::vector<SearchNode> m_nodes;

        parameters::SearchParameters m_parameters;

        std::condition_variable& m_cv;
        std::unique_lock<std::mutex>& m_lock;
        bool& m_best_move_available;
        transposition_table::TranspositionTable& m_transposition_table;
    };
}
