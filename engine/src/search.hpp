#pragma once

#include <vector>
#include <condition_variable>
#include <mutex>

#include "game.hpp"
#include "search_node.hpp"
#include "evaluation.hpp"

namespace search {
    class Search {
    public:
        Search(
            std::condition_variable& cv,
            std::unique_lock<std::mutex>& lock,
            bool& result_available,
            int parameter_piece,
            int parameter_depth
        );

        ~Search() = default;

        Search(const Search&) = delete;
        Search& operator=(const Search&) = delete;
        Search(Search&&) = delete;
        Search& operator=(Search&&) = delete;

        game::Move search(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );

        bool* get_should_stop() { return &should_stop; }
    private:
        evaluation::Eval minimax(
            unsigned int depth,
            unsigned int plies_from_root,
            SearchNode& current_node
        );

        SearchNode& setup_nodes(
            const game::Position& position,
            const std::vector<game::Position>& previous_positions,
            const std::vector<game::Move>& moves_played
        );

        bool is_advancement(const game::Position& position, const game::Move& move);
        void notify_result_available();

        bool notified_result_available {false};
        bool should_stop {false};
        unsigned int nodes_evaluated {};
        game::Move best_move {};

        // The current position and previous positions (for threefold reptition)
        // position0, position1, position2, ..., positionN (current)
        std::vector<SearchNode> nodes;

        evaluation::Parameters parameters;

        std::condition_variable& cv;
        std::unique_lock<std::mutex>& lock;
        bool& result_available;
    };
}
