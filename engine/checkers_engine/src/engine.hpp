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
#include "book.hpp"

namespace engine {
    class Engine {
    public:
        // Commands the engine executes
        // May throw errors, that are caught and ignored in the loop
        void init();
        void newgame(const std::optional<std::string>& position, const std::optional<std::vector<std::string>>& moves);
        void move(const std::string& move);
        void go(const std::optional<std::string>& max_depth, const std::optional<std::string>& max_time, bool dont_play_move);
        void stop();
        void getparameters() const;
        void getparameter(const std::string& name) const;
        void setparameter(const std::string& name, const std::string& value);
        void quit();
        void getname() const;
        void board() const;
    private:
        game::Move search_move() noexcept;
        bool lookup_book();
        void reset_position(const std::string& fen_string);
        void initialize_parameters();
        void ignore_invalid_command_on_init(bool after_init = false) const;

        std::thread m_thread;
        std::condition_variable m_cv;
        std::mutex m_mutex;
        parameters::Parameters m_parameters;
        transposition_table::TranspositionTable m_transposition_table;
        book::Book m_opening_book;

        // Thread flag; set to true on initialization
        bool m_running {false};

        // Thread flag; set to true when there is something to search or when the engine needs to stop
        bool m_search {false};

        // Thread flag; set to true when m_should_stop is set
        bool m_instance_ready {false};

        // Number representing a particular search
        int m_search_sequence {0};

        // Internal position (current)
        game::GamePosition m_position;

        // position0, position1, position2, ..., positionN (most recent)
        std::vector<game::GamePosition> m_previous_positions;

        // move0, move1, move2, ..., moveN (most recent)
        std::vector<game::Move> m_moves_played;

        // Setup at every instance invocation
        bool* m_should_stop {nullptr};

        // Variables used by the search, must be reset every time before the search
        struct SearchOptions {
            bool dont_play_move {};
            int max_depth {};
            double max_time {};
        } m_search_options;
    };
}
