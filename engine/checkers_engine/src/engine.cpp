#include "engine.hpp"

#include <stdexcept>
#include <cassert>

#include "moves.hpp"
#include "messages.hpp"
#include "search.hpp"
#include "error.hpp"

namespace engine {
    static const char* START_POSITION {"B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32"};

    static void reset_position(Engine& engine, const std::string& fen_string) {
        game::set_position(engine.game.position, fen_string);

        engine.game.position.plies = 0;
        engine.game.position.plies_without_advancement = 0;

        engine.game.previous_positions.clear();
        engine.game.moves_played.clear();
    }

    static void initialize_parameters(Engine& engine) {
        engine.minimax.parameters["piece"] = 10;
        engine.minimax.parameters["depth"] = 4;
    }

    static void ignore_invalid_command_on_init(const Engine& engine, bool after_init = false) {
        const bool command_invalid {
            after_init ? engine.minimax.running : !engine.minimax.running
        };

        if (command_invalid) {
            throw error::InvalidCommand();
        }
    }

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
        ignore_invalid_command_on_init(*this, true);

        minimax.running = true;

        minimax.thread = std::thread([this]() {
            while (true) {
                // Check this condition also before wait
                if (!minimax.running) {
                    break;
                }

                // Wait for some work to do or to exit the loop
                std::unique_lock<std::mutex> lock {minimax.mutex};
                minimax.cv.wait(lock, [this]() { return static_cast<bool>(minimax.search_function); });

                if (!minimax.running) {
                    break;
                }

                // Do the actual work now
                // Search returns a valid result or nothing, if the game is over
                const auto [best_move, dont_play] {minimax.search_function(lock)};

                if (!dont_play) {
                    if (best_move) {
                        moves::play_move(game.position, *best_move);

                        game.previous_positions.push_back(game.position);
                        game.moves_played.push_back(*best_move);
                    }
                }

                // Reset the search function as a signal for the cv
                minimax.search_function = {};

                // Message the GUI only now, to indicate that we are ready for another GO
                // Best move is already something or nothing
                messages::bestmove(best_move);
            }
        });

        reset_position(*this, START_POSITION);

        // Store the initial position too
        game.previous_positions.push_back(game.position);

        // Parameters must have default values at this stage
        initialize_parameters(*this);
    }

    void Engine::newgame(const std::optional<std::string>& position, const std::optional<std::vector<std::string>>& moves) {
        ignore_invalid_command_on_init(*this);

        if (position) {
            reset_position(*this, *position);
        } else {
            reset_position(*this, START_POSITION);
        }

        // Store the initial position too, as it can be any specific position
        game.previous_positions.push_back(game.position);

        if (moves) {
            // Play the moves and store the positions and moves (for threefold repetition)
            for (const std::string& move : *moves) {
                game::make_move(game.position, move);

                game.previous_positions.push_back(game.position);
                game.moves_played.push_back(game::parse_move_string(move));
            }
        }
    }

    void Engine::move(const std::string& move) {
        ignore_invalid_command_on_init(*this);

        game::make_move(game.position, move);

        game.previous_positions.push_back(game.position);
        game.moves_played.push_back(game::parse_move_string(move));
    }

    void Engine::go(bool dont_play_move) {
        ignore_invalid_command_on_init(*this);

        if (minimax.search_function) {
            // Ignore invalid
            return;
        }

        // Set to true when a best move is set
        bool result_available {false};

        const auto search_function {
            [this, dont_play_move, &result_available](std::unique_lock<std::mutex>& lock) {
                search::Search instance {
                    minimax.cv,
                    lock,
                    result_available,
                    std::get<0>(minimax.parameters.at("piece")),
                    std::get<0>(minimax.parameters.at("depth"))
                };

                minimax.should_stop = instance.get_should_stop();

                auto previous_positions {game.previous_positions};
                previous_positions.pop_back();

                const auto best_move {
                    instance.search(game.position, previous_positions, game.moves_played)
                };

                // Must reset this back to null here, after the search
                minimax.should_stop = nullptr;

                return std::make_pair(best_move, dont_play_move);
            }
        };

        // Set the search fuction; it's a signal for the cv
        {
            std::lock_guard<std::mutex> lock {minimax.mutex};
            minimax.search_function = search_function;
        }
        minimax.cv.notify_one();

        // Wait for the first result to become available; thus the engine cannot process a stop command and thus
        // the resulting move must be valid, or the game must be over
        {
            std::unique_lock<std::mutex> lock {minimax.mutex};
            minimax.cv.wait(lock, [&result_available]() { return result_available; });
        }
    }

    void Engine::stop() {
        ignore_invalid_command_on_init(*this);

        if (minimax.should_stop != nullptr) {
            *minimax.should_stop = true;
        }
    }

    void Engine::getparameters() const {
        ignore_invalid_command_on_init(*this);

        messages::parameters(minimax.parameters);
    }

    void Engine::getparameter(const std::string& name) const {
        ignore_invalid_command_on_init(*this);

        const auto iter {minimax.parameters.find(name)};

        if (iter == minimax.parameters.cend()) {
            return;
        }

        messages::parameter(name, iter->second);
    }

    void Engine::setparameter(const std::string& name, const std::string& value) {
        ignore_invalid_command_on_init(*this);

        auto iter {minimax.parameters.find(name)};

        if (iter == minimax.parameters.cend()) {
            return;
        }

        Parameter& parameter {iter->second};

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

        if (!minimax.running) {
            // There is nothing to do; the main loop handles the rest of the uninitialization
            return;
        }

        if (minimax.search_function) {
            // The thread is already busy searching, just join it

            minimax.running = false;

            minimax.thread.join();
        } else {
            // Set dummy work to wake up the thread from sleeping
            minimax.search_function = [](std::unique_lock<std::mutex>&) -> SearchResult { return {}; };
            minimax.running = false;

            minimax.cv.notify_one();
            minimax.thread.join();
        }
    }

    void Engine::getname() const {
        ignore_invalid_command_on_init(*this);

        messages::name();
    }
}
