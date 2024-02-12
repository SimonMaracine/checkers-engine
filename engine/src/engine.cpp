#include "engine.hpp"

#include <cassert>
#include <stdexcept>

#include "moves.hpp"
#include "messages.hpp"
#include "search.hpp"
#include "error.hpp"

namespace engine {
    static void reset_position(EngineData& data, const std::string& fen_string) {
        game::set_position(data.game.position.position, fen_string);

        data.game.position.plies = 0u;
        data.game.position.plies_without_advancement = 0u;

        data.game.previous_positions.clear();
        data.game.moves_played.clear();
    }

    static void wait_sleeping(EngineData& data) {
        std::unique_lock<std::mutex> lock {data.minimax.mutex};
        data.minimax.cv.wait(lock, [&data]() { return static_cast<bool>(data.minimax.search); });
    }

    static void initialize_parameters(EngineData& data) {
        data.minimax.parameters["piece"] = 10;
    }

    static int parse_int(const std::string& string) {
        int result {0};

        try {
            result = std::stoi(string);
        } catch (const std::invalid_argument&) {
            throw error::ERR;
        } catch (const std::out_of_range&) {
            throw error::ERR;
        }

        return result;
    }

    void init(engine::EngineData& data) {
        assert(!data.minimax.running);

        data.minimax.running = true;

        data.minimax.thread = std::thread([&data]() {
            while (true) {
                // Returning from this only happens when there is work to do or when it should stop running
                wait_sleeping(data);

                if (!data.minimax.running) {
                    break;
                }

                const auto [best_move, dont_play] {data.minimax.search()};

                messages::bestmove(best_move);

                if (!dont_play) {
                    moves::play_move(data.game.position, best_move);

                    data.game.previous_positions.push_back(data.game.position);
                    data.game.moves_played.push_back(best_move);
                }

                // Reset the function as a signal for the cv
                data.minimax.search = {};
            }
        });

        reset_position(data, "B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32");

        // Store the initial position too
        data.game.previous_positions.push_back(data.game.position);

        initialize_parameters(data);
    }

    void newgame(
        engine::EngineData& data,
        const std::optional<std::string>& position,
        const std::optional<std::vector<std::string>>& moves
    ) {
        if (position) {
            reset_position(data, *position);
        } else {
            reset_position(data, "B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32");
        }

        // Store the initial position too, as it can be any specific position
        data.game.previous_positions.push_back(data.game.position);

        if (moves) {
            // Play the moves and store the positions and moves for threefold repetition
            for (const std::string& move : *moves) {
                game::make_move(data.game.position, move);

                data.game.previous_positions.push_back(data.game.position);
                data.game.moves_played.push_back(game::parse_move(move));
            }
        }
    }

    void move(engine::EngineData& data, const std::string& move) {
        game::make_move(data.game.position, move);

        data.game.previous_positions.push_back(data.game.position);
        data.game.moves_played.push_back(game::parse_move(move));
    }

    void go(engine::EngineData& data, bool dont_play_move) {
        assert(data.minimax.running);  // TODO maybe use exceptions instead
        assert(!data.minimax.search);

        data.minimax.search = [&data, dont_play_move]() {
            const int parameter_piece {std::get<0u>(data.minimax.parameters.at("piece"))};

            search::Search instance {parameter_piece};

            auto previous_positions {data.game.previous_positions};
            previous_positions.pop_back();

            const game::Move best_move {
                instance.search(data.game.position, previous_positions, data.game.moves_played)
            };

            return std::make_pair(best_move, dont_play_move);
        };

        data.minimax.cv.notify_one();
    }

    void getparameters(engine::EngineData& data) {
        messages::parameters(data.minimax.parameters);
    }

    void getparameter(engine::EngineData& data, const std::string& name) {
        messages::parameter(name, data.minimax.parameters.at(name));
    }

    void setparameter(engine::EngineData& data, const std::string& name, const std::string& value) {
        const Param& parameter {data.minimax.parameters.at(name)};

        switch (parameter.index()) {
            case 0u:
                data.minimax.parameters[name] = parse_int(value);
                break;
            default:
                assert(false);
                break;
        }
    }

    void quit(engine::EngineData& data) {
        assert(data.minimax.running);
        assert(!data.minimax.search);

        // Set dummy work to wake up the thread from sleeping
        data.minimax.search = []() -> SearchResult { return {}; };
        data.minimax.running = false;

        data.minimax.cv.notify_one();
        data.minimax.thread.join();
    }
}
