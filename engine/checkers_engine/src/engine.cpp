#include "engine.hpp"

#include <stdexcept>
#include <cassert>

#include "moves.hpp"
#include "messages.hpp"
#include "search.hpp"
#include "error.hpp"

namespace engine {
    static const char* START_POSITION {"B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32"};

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

        m_running = true;

        m_thread = std::thread([this]() {
            while (true) {
                // Check this condition also before wait
                if (!m_running) {
                    break;
                }

                // Wait for some work to do or to exit the loop
                std::unique_lock<std::mutex> lock {m_mutex};
                m_cv.wait(lock, [this]() { return m_search; });

                if (!m_running) {
                    break;
                }

                // Do the actual work now
                // Search returns a valid result or nothing, if the game is over
                const auto best_move {search_move(lock)};

                if (!m_dont_play_move && best_move) {
                    moves::play_move(m_position, *best_move);

                    m_previous_positions.push_back(m_position);
                    m_moves_played.push_back(*best_move);
                }

                // Reset the search flag as a signal for the cv
                m_search = false;

                // Message the GUI only now, to indicate that we are ready for another GO
                // Best move is already something or nothing
                messages::bestmove(best_move);
            }
        });

        reset_position(START_POSITION);

        // Store the initial position too
        m_previous_positions.push_back(m_position);

        // Parameters must have default values at this stage
        initialize_parameters();
    }

    void Engine::newgame(const std::optional<std::string>& position, const std::optional<std::vector<std::string>>& moves) {
        ignore_invalid_command_on_init();

        if (position) {
            reset_position(*position);
        } else {
            reset_position(START_POSITION);
        }

        // Store the initial position too, as it can be any specific position
        m_previous_positions.push_back(m_position);

        if (moves) {
            // Play the moves and store the positions and moves (for threefold repetition)
            for (const std::string& move : *moves) {
                game::play_move(m_position, move);

                m_previous_positions.push_back(m_position);
                m_moves_played.push_back(game::parse_move_string(move));
            }
        }
    }

    void Engine::move(const std::string& move) {
        ignore_invalid_command_on_init();

        game::play_move(m_position, move);

        m_previous_positions.push_back(m_position);
        m_moves_played.push_back(game::parse_move_string(move));
    }

    void Engine::go(bool dont_play_move) {
        ignore_invalid_command_on_init();

        if (m_search) {
            // Ignore invalid
            return;
        }

        // Set if the resulted move should be played or not
        m_dont_play_move = dont_play_move;

        // Set the search flag; it's a signal for the cv
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            m_search = true;
        }
        m_cv.notify_one();

        // Wait for the first result to become available; thus the engine cannot process a stop command and thus
        // the resulting move must be valid, or the game must be over
        {
            std::unique_lock<std::mutex> lock {m_mutex};
            m_cv.wait(lock, [this]() { return m_best_move_available; });
        }

        // Best move flag must be reset after use
        m_best_move_available = false;
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

        if (iter == m_parameters.cend()) {
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

        if (m_search) {
            // The thread is already busy searching, just join it

            m_running = false;
            m_thread.join();
        } else {
            // Set dummy work to wake up the thread from sleeping
            m_search = true;
            m_running = false;

            m_cv.notify_one();
            m_thread.join();
        }
    }

    void Engine::getname() const {
        ignore_invalid_command_on_init();

        messages::name();
    }

    std::optional<game::Move> Engine::search_move(std::unique_lock<std::mutex>& lock) {
        search::Search instance {
            m_cv,
            lock,
            m_best_move_available,
            m_parameters
        };

        m_should_stop = instance.get_should_stop();

        auto previous_positions {m_previous_positions};
        previous_positions.pop_back();

        const auto best_move {instance.search(
            m_position,
            previous_positions,
            m_moves_played,
            static_cast<unsigned int>(std::get<0>(m_parameters.at("depth")))
        )};

        // Must reset this back to null here, after the search
        m_should_stop = nullptr;

        return best_move;
    }

    void Engine::reset_position(const std::string& fen_string) {
        game::set_position(m_position, fen_string);

        m_position.plies = 0;
        m_position.plies_without_advancement = 0;

        m_previous_positions.clear();
        m_moves_played.clear();
    }

    void Engine::initialize_parameters() {
        m_parameters["piece"] = 10;
        m_parameters["depth"] = 4;
    }

    void Engine::ignore_invalid_command_on_init(bool after_init) const {
        const bool command_invalid {
            after_init ? m_running : !m_running
        };

        if (command_invalid) {
            throw error::InvalidCommand();
        }
    }
}
