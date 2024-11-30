#pragma once

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <string>
#include <optional>

#include "game.hpp"
#include "parameters.hpp"
#include "transposition_table.hpp"

namespace engine {
    class Engine {
    public:
        // Commands the engine executes
        // May throw errors, that are caught and ignored in the loop
        void init();
        void newgame(const std::optional<std::string>& position, const std::optional<std::vector<std::string>>& moves);
        void move(const std::string& move);
        void go(bool dont_play_move);
        void stop();
        void getparameters() const;
        void getparameter(const std::string& name) const;
        void setparameter(const std::string& name, const std::string& value);
        void quit();
        void getname() const;
        void board() const;
    private:
        std::optional<game::Move> search_move(std::unique_lock<std::mutex>& lock);
        void reset_position(const std::string& fen_string);
        void initialize_parameters();
        void ignore_invalid_command_on_init(bool after_init = false) const;

        /* TODO
            opening book
            other
        */

        std::thread m_thread;
        std::condition_variable m_cv;
        std::mutex m_mutex;
        parameters::Parameters m_parameters;
        transposition_table::TranspositionTable m_transposition_table;

        // Setup at every instance invocation
        bool* m_should_stop {nullptr};

        // Thread flag; set to true on initialization
        bool m_running {false};

        // Thread flag; set to true when there is something to search or when the engine needs to stop
        bool m_search {false};

        // Thread flag; set to true when the first best move is found
        bool m_best_move_available {false};

        // Flag used after a search is complete; must be reset every time
        bool m_dont_play_move {};

        // Internal position (current)
        game::Position m_position;

        // position0, position1, position2, ..., positionN (most recent)
        std::vector<game::Position> m_previous_positions;

        // move0, move1, move2, ..., moveN (most recent)
        std::vector<game::Move> m_moves_played;
    };
}
