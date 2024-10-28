#include "engine.hpp"

#include <stdexcept>
#include <cassert>

#include "moves.hpp"
#include "messages.hpp"
#include "search.hpp"
#include "error.hpp"

namespace engine {
    static const char* START_POSITION {"B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32"};

    static void reset_position(EngineData& data, const std::string& fen_string) {
        game::set_position(data.game.position, fen_string);

        data.game.position.plies = 0;
        data.game.position.plies_without_advancement = 0;

        data.game.previous_positions.clear();
        data.game.moves_played.clear();
    }

    static void initialize_parameters(EngineData& data) {
        data.minimax.parameters["piece"] = 10;
        data.minimax.parameters["depth"] = 4;
    }

    static void ignore_invalid_command_on_init(const EngineData& data, bool after_init = false) {
        const bool command_invalid {
            after_init ? data.minimax.running : !data.minimax.running
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

    void init(EngineData& data) {
        ignore_invalid_command_on_init(data, true);

        data.minimax.running = true;

        data.minimax.thread = std::thread([&data]() {
            while (true) {
                // Check this condition also before wait
                if (!data.minimax.running) {
                    break;
                }

                // Wait for some work to do or to exit the loop
                std::unique_lock<std::mutex> lock {data.minimax.mutex};
                data.minimax.cv.wait(lock, [&data]() { return static_cast<bool>(data.minimax.search_function); });

                if (!data.minimax.running) {
                    break;
                }

                // Do the actual work now
                // Search returns a valid result or nothing, if the game is over
                const auto [best_move, dont_play] {data.minimax.search_function(lock)};

                if (!dont_play) {
                    if (best_move) {
                        moves::play_move(data.game.position, *best_move);

                        data.game.previous_positions.push_back(data.game.position);
                        data.game.moves_played.push_back(*best_move);
                    }
                }

                // Reset the search function as a signal for the cv
                data.minimax.search_function = {};

                // Message the GUI only now, to indicate that we are ready for another GO
                // Best move is already something or nothing
                messages::bestmove(best_move);
            }
        });

        reset_position(data, START_POSITION);

        // Store the initial position too
        data.game.previous_positions.push_back(data.game.position);

        // Parameters must have default values at this stage
        initialize_parameters(data);
    }

    void newgame(EngineData& data, const std::optional<std::string>& position, const std::optional<std::vector<std::string>>& moves) {
        ignore_invalid_command_on_init(data);

        if (position) {
            reset_position(data, *position);
        } else {
            reset_position(data, START_POSITION);
        }

        // Store the initial position too, as it can be any specific position
        data.game.previous_positions.push_back(data.game.position);

        if (moves) {
            // Play the moves and store the positions and moves (for threefold repetition)
            for (const std::string& move : *moves) {
                game::make_move(data.game.position, move);

                data.game.previous_positions.push_back(data.game.position);
                data.game.moves_played.push_back(game::parse_move_string(move));
            }
        }
    }

    void move(EngineData& data, const std::string& move) {
        ignore_invalid_command_on_init(data);

        game::make_move(data.game.position, move);

        data.game.previous_positions.push_back(data.game.position);
        data.game.moves_played.push_back(game::parse_move_string(move));
    }

    void go(EngineData& data, bool dont_play_move) {
        ignore_invalid_command_on_init(data);

        if (data.minimax.search_function) {
            // Ignore invalid
            return;
        }

        // Set to true when a best move is set
        bool result_available {false};

        const auto search_function {
            [&data, dont_play_move, &result_available](std::unique_lock<std::mutex>& lock) {
                search::Search instance {
                    data.minimax.cv,
                    lock,
                    result_available,
                    std::get<0>(data.minimax.parameters.at("piece")),
                    std::get<0>(data.minimax.parameters.at("depth"))
                };

                data.minimax.should_stop = instance.get_should_stop();

                auto previous_positions {data.game.previous_positions};
                previous_positions.pop_back();

                const auto best_move {
                    instance.search(data.game.position, previous_positions, data.game.moves_played)
                };

                // Must reset this back to null here, after the search
                data.minimax.should_stop = nullptr;

                return std::make_pair(best_move, dont_play_move);
            }
        };

        // Set the search fuction; it's a signal for the cv
        {
            std::lock_guard<std::mutex> lock {data.minimax.mutex};
            data.minimax.search_function = search_function;
        }
        data.minimax.cv.notify_one();

        // Wait for the first result to become available; thus the engine cannot process a stop command and thus
        // the resulting move must be valid, or the game must be over
        {
            std::unique_lock<std::mutex> lock {data.minimax.mutex};
            data.minimax.cv.wait(lock, [&result_available]() { return result_available; });
        }
    }

    void stop(EngineData& data) {
        ignore_invalid_command_on_init(data);

        if (data.minimax.should_stop != nullptr) {
            *data.minimax.should_stop = true;
        }
    }

    void getparameters(const EngineData& data) {
        ignore_invalid_command_on_init(data);

        messages::parameters(data.minimax.parameters);
    }

    void getparameter(const EngineData& data, const std::string& name) {
        ignore_invalid_command_on_init(data);

        const auto iter {data.minimax.parameters.find(name)};

        if (iter == data.minimax.parameters.cend()) {
            return;
        }

        messages::parameter(name, iter->second);
    }

    void setparameter(EngineData& data, const std::string& name, const std::string& value) {
        ignore_invalid_command_on_init(data);

        auto iter {data.minimax.parameters.find(name)};

        if (iter == data.minimax.parameters.cend()) {
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

    void quit(EngineData& data) {
        // Must not throw exeptions

        if (!data.minimax.running) {
            // There is nothing to do; the main loop handles the rest of the uninitialization
            return;
        }

        if (data.minimax.search_function) {
            // The thread is already busy searching, just join it

            data.minimax.running = false;

            data.minimax.thread.join();
        } else {
            // Set dummy work to wake up the thread from sleeping
            data.minimax.search_function = [](std::unique_lock<std::mutex>&) -> SearchResult { return {}; };
            data.minimax.running = false;

            data.minimax.cv.notify_one();
            data.minimax.thread.join();
        }
    }
}
