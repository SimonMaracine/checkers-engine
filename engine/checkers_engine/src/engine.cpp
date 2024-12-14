#include "engine.hpp"

#include <utility>
#include <limits>
#include <cstdint>
#include <cassert>

#include "moves.hpp"
#include "messages.hpp"
#include "search.hpp"
#include "error.hpp"
#include "zobrist.hpp"

// https://en.cppreference.com/w/cpp/thread/condition_variable

// Check for 64-bit architecture
#if SIZE_MAX != 0xFFFFFFFFFFFFFFFFul
    #error "No support for 32-bit"
#endif

namespace engine {
    static const char* START_POSITION {"B:B1,2,3,4,5,6,7,8,9,10,11,12:W21,22,23,24,25,26,27,28,29,30,31,32"};

    static int parse_int(const std::string& string) {
        try {
            return std::stoi(string);
        } catch (const std::invalid_argument&) {
            throw error::InvalidCommand();
        } catch (const std::out_of_range&) {
            throw error::InvalidCommand();
        }
    }

    static float parse_float(const std::string& string) {
        try {
            return std::stof(string);
        } catch (const std::invalid_argument&) {
            throw error::InvalidCommand();
        } catch (const std::out_of_range&) {
            throw error::InvalidCommand();
        }
    }

    static double parse_double(const std::string& string) {
        try {
            return std::stod(string);
        } catch (const std::invalid_argument&) {
            throw error::InvalidCommand();
        } catch (const std::out_of_range&) {
            throw error::InvalidCommand();
        }
    }

    static bool parse_bool(const std::string& string) {
        if (string == "true") {
            return true;
        } else if (string == "false") {
            return false;
        } else {
            throw error::InvalidCommand();
        }
    }

    void Engine::init() {
        ignore_invalid_command_on_init(true);

        try {
            // TT is empty by default
            m_transposition_table.allocate(transposition_table::mib_to_bytes(750));
        } catch (const std::bad_alloc&) {
            throw error::Fatal();
        }

        m_running = true;

        m_thread = std::thread([this]() {
            while (true) {
                // Wait for some work to do or to exit the loop
                std::unique_lock<std::mutex> lock {m_mutex};
                m_cv.wait(lock, [this]() { return m_search || !m_running; });

                if (!m_running) {
                    break;
                }

                // Do the actual work now
                // Search returns a valid result or nothing, if the game is over
                const game::Move best_move {search_move()};

                if (!m_search_options.dont_play_move && best_move != game::NULL_MOVE) {
                    m_previous_positions.push_back(m_position);
                    m_moves_played.push_back(best_move);

                    game::play_move(m_position, best_move);
                }

                // Reset the search flag as a signal for the cv; the lock is still being held
                m_search = false;

                // Message the GUI only now, to indicate that we are ready for another GO
                // Best move is already something or nothing
                messages::bestmove(best_move);
            }
        });

        // Parameters must have default values at this stage
        initialize_parameters();

        // Zobrist hash is static
        zobrist::instance.initialize();

        // Reset position only after Zobrist hashing is initialized
        reset_position(START_POSITION);
    }

    void Engine::newgame(const std::optional<std::string>& position, const std::optional<std::vector<std::string>>& moves) {
        ignore_invalid_command_on_init();

        // Don't share data between games
        m_transposition_table.clear();

        // Resetting the sequence is not really important, but it's a good idea
        m_search_sequence = 0;

        if (position) {
            reset_position(*position);
        } else {
            reset_position(START_POSITION);
        }

        if (moves) {
            // State needs to be restored in case of error
            auto backup_previous_positions {m_previous_positions};
            auto backup_moves_played {m_moves_played};

            // Play the moves and store the positions and moves (for threefold repetition)
            for (const std::string& move : *moves) {
                m_previous_positions.push_back(m_position);
                m_moves_played.push_back(game::parse_move_string(move));

                try {
                    game::play_move(m_position, move);
                } catch (error::InvalidCommand) {
                    m_previous_positions = std::move(backup_previous_positions);
                    m_moves_played = std::move(backup_moves_played);
                    throw;
                }
            }
        }
    }

    void Engine::move(const std::string& move) {
        ignore_invalid_command_on_init();

        // State needs to be restored in case of error
        auto backup_previous_positions {m_previous_positions};
        auto backup_moves_played {m_moves_played};

        m_previous_positions.push_back(m_position);
        m_moves_played.push_back(game::parse_move_string(move));

        try {
            game::play_move(m_position, move);
        } catch (error::InvalidCommand) {
            m_previous_positions = std::move(backup_previous_positions);
            m_moves_played = std::move(backup_moves_played);
            throw;
        }
    }

    void Engine::go(const std::optional<std::string>& max_depth, const std::optional<std::string>& max_time, bool dont_play_move) {
        ignore_invalid_command_on_init();

        if (m_search) {
            throw error::InvalidCommand();
        }

        // Set the options before every search
        m_search_options.max_depth = max_depth ? parse_int(*max_depth) : game::MAX_DEPTH;
        m_search_options.max_time = max_time ? parse_double(*max_time) : std::numeric_limits<double>::max();
        m_search_options.dont_play_move = dont_play_move;

        // Set the search flag; it's a signal for the cv
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            m_search = true;
        }
        m_cv.notify_one();

        // Wait only for m_should_stop pointer to become available
        // If we send then a STOP command, it will be processed and the search will stop at its will
        {
            std::unique_lock<std::mutex> lock {m_mutex};
            m_cv.wait(lock, [this]() { return m_instance_ready; });
        }

        // Flag must be reset after use
        m_instance_ready = false;
    }

    void Engine::stop() {
        ignore_invalid_command_on_init();

        if (m_should_stop != nullptr) {
            *m_should_stop = true;
        }
    }

    void Engine::getparameters() const {
        ignore_invalid_command_on_init();

        messages::parameters(m_parameters);
    }

    void Engine::getparameter(const std::string& name) const {
        ignore_invalid_command_on_init();

        const auto iter {m_parameters.find(name)};

        if (iter == m_parameters.cend()) {
            return;
        }

        messages::parameter(name, iter->second);
    }

    void Engine::setparameter(const std::string& name, const std::string& value) {
        ignore_invalid_command_on_init();

        auto iter {m_parameters.find(name)};

        if (iter == m_parameters.end()) {
            return;
        }

        parameters::Parameter& parameter {iter->second};

        switch (parameter.index()) {
            case 0:
                parameter = parse_int(value);
                break;
            case 1:
                parameter = parse_float(value);
                break;
            case 2:
                parameter = parse_bool(value);
                break;
            case 3:
                parameter = value;
                break;
            default:
                assert(false);
                break;
        }
    }

    void Engine::quit() {
        // Must not throw exceptions

        if (!m_running) {
            // There is nothing to do; the main loop handles the rest of the uninitialization
            return;
        }

        // Do what stop would do
        if (m_should_stop != nullptr) {
            *m_should_stop = true;
        }

        // Wake up the thread from sleeping, a signal for the cv
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            m_running = false;
        }
        m_cv.notify_one();

        m_thread.join();
    }

    void Engine::getname() const {
        ignore_invalid_command_on_init();

        messages::name();
    }

    void Engine::board() const {
        ignore_invalid_command_on_init();

        messages::board(m_position);
    }

    game::Move Engine::search_move() noexcept {
        m_search_sequence++;

        search::Search instance {m_search_sequence, m_parameters, m_transposition_table};

        m_should_stop = instance.get_should_stop();
        m_instance_ready = true;

        // Notify now that the search instance is ready
        m_mutex.unlock();
        m_cv.notify_one();

        const game::Move best_move {instance.search(
            m_position,
            m_previous_positions,
            m_moves_played,
            m_search_options.max_depth,
            m_search_options.max_time
        )};

        // Must reset this back to null here after the search, because it will soon be invalidated
        m_should_stop = nullptr;

        return best_move;
    }

    void Engine::reset_position(const std::string& fen_string) {
        game::set_position(m_position, fen_string);
        m_previous_positions.clear();
        m_moves_played.clear();
    }

    void Engine::initialize_parameters() {
        m_parameters["material_pawn"] = 24;
        m_parameters["material_king"] = 56;
        m_parameters["positioning_pawn"] = 1;
        m_parameters["positioning_king"] = 3;
        m_parameters["crowdness"] = 1;
    }

    void Engine::ignore_invalid_command_on_init(bool after_init) const {
        const bool command_invalid {after_init ? m_running : !m_running};

        if (command_invalid) {
            throw error::InvalidCommand();
        }
    }
}
