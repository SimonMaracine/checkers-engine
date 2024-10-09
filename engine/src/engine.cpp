#include "engine.hpp"

#include <cassert>
#include <stdexcept>

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

    static int parse_int(const std::string& string) {
        int result {0};

        try {
            result = std::stoi(string);
        } catch (const std::invalid_argument&) {
            throw error::Error();
        } catch (const std::out_of_range&) {
            throw error::Error();
        }

        return result;
    }

    void init(EngineData& data) {
        if (data.minimax.running) {
            throw error::Error();
        }

        data.minimax.running = true;

        data.minimax.thread = std::thread([&data]() {
            while (true) {
                // Check this condition also before wait
                if (!data.minimax.running) {
                    break;
                }

                // Wait for some work to do or to exit the loop
                std::unique_lock<std::mutex> lock {data.minimax.mutex};
                data.minimax.cv.wait(lock, [&data]() { return static_cast<bool>(data.minimax.search_func); });

                if (!data.minimax.running) {
                    break;
                }

                const auto [best_move, dont_play] {data.minimax.search_func(lock)};

                if (!dont_play) {
                    moves::play_move(data.game.position, best_move);

                    data.game.previous_positions.push_back(data.game.position);
                    data.game.moves_played.push_back(best_move);
                }

                // Reset the search function as a signal for the cv
                data.minimax.search_func = {};

                // Message the GUI only now, to indicate that we are ready for another GO
                messages::bestmove(best_move);
            }
        });

        reset_position(data, START_POSITION);

        // Store the initial position too
        data.game.previous_positions.push_back(data.game.position);

        // Parameters must have default values at this stage
        initialize_parameters(data);
    }

    void newgame(
        EngineData& data,
        const std::optional<std::string>& position,
        const std::optional<std::vector<std::string>>& moves
    ) {
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
                data.game.moves_played.push_back(game::parse_move(move));
            }
        }
    }

    void move(EngineData& data, const std::string& move) {
        game::make_move(data.game.position, move);

        data.game.previous_positions.push_back(data.game.position);
        data.game.moves_played.push_back(game::parse_move(move));
    }

    void go(EngineData& data, bool dont_play_move) {
        if (!data.minimax.running) {
            throw error::Error();
        }

        if (data.minimax.search_func) {
            // Just ignore this GO command
            // Not great, but it's okay since it's UB for GUI to send GO at this point
            return;  // TODO make sure that the engine actually follows the protocol
        }

        // Set to true when a best move is set
        bool result_available {false};

        const auto search_func {
            [&data, dont_play_move, &result_available](auto& lock) {
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

                const game::Move best_move {
                    instance.search(data.game.position, previous_positions, data.game.moves_played)
                };

                assert(!game::is_move_invalid(best_move));  // FIXME this failed at the very end of a game

                // Must reset this back to null
                data.minimax.should_stop = nullptr;

                return std::make_pair(best_move, dont_play_move);
            }
        };

        // Set the search fuction; it's a signal for the cv
        {
            std::lock_guard<std::mutex> lock {data.minimax.mutex};
            data.minimax.search_func = search_func;
        }
        data.minimax.cv.notify_one();

        // Wait for the first result to become available
        {
            std::unique_lock<std::mutex> lock {data.minimax.mutex};
            data.minimax.cv.wait(lock, [&result_available]() { return result_available; });
        }
    }

    void stop(EngineData& data) {
        if (data.minimax.should_stop != nullptr) {
            *data.minimax.should_stop = true;
        }
    }

    void getparameters(const EngineData& data) {
        messages::parameters(data.minimax.parameters);
    }

    void getparameter(const EngineData& data, const std::string& name) {
        const auto iter {data.minimax.parameters.find(name)};

        if (iter == data.minimax.parameters.cend()) {
            return;
        }

        messages::parameter(name, iter->second);
    }

    void setparameter(EngineData& data, const std::string& name, const std::string& value) {
        auto iter {data.minimax.parameters.find(name)};

        if (iter == data.minimax.parameters.cend()) {
            return;
        }

        Param& parameter {iter->second};

        switch (parameter.index()) {
            case 0:
                parameter = parse_int(value);
                break;
            default:
                assert(false);
                break;
        }
    }

    void quit(EngineData& data) {
        if (!data.minimax.running) {
            throw error::Error();
        }

        if (data.minimax.search_func) {
            // The thread is already busy searching, just join it

            data.minimax.running = false;

            data.minimax.thread.join();
        } else {
            // Set dummy work to wake up the thread from sleeping
            data.minimax.search_func = [](auto&) -> SearchResult { return {}; };
            data.minimax.running = false;

            data.minimax.cv.notify_one();
            data.minimax.thread.join();
        }
    }
}
