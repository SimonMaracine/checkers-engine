#pragma once

#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <variant>
#include <string>
#include <optional>
#include <utility>

#include "game.hpp"

namespace engine {
    using Param = std::variant<int>;
    using SearchResult = std::pair<game::Move, bool>;

    struct EngineData {
        /* TODO
            transposition table
            opening book
            other
        */

       struct {
            std::thread thread;
            std::function<SearchResult(std::unique_lock<std::mutex>&)> search;
            std::condition_variable cv;
            std::mutex mutex;
            std::unordered_map<std::string, Param> parameters;

            // Setup at every instance invocation
            bool* should_stop {nullptr};

            // Thread flag
            bool running {false};
       } minimax;

        struct {
            // Internal position
            game::Position position;

            // position0, position1, position2, ..., positionN (current)
            // This always stores the current position, which needs to be dropped when passed to the search function
            std::vector<game::Position> previous_positions;

            // move0, move1, move2, ..., moveN (most recent)
            std::vector<game::Move> moves_played;
        } game;
    };

    // Commands the engine executes
    // Arguments to these aren't checked
    // May throw errors
    void init(engine::EngineData& data);
    void newgame(
        engine::EngineData& data,
        const std::optional<std::string>& position,
        const std::optional<std::vector<std::string>>& moves
    );
    void move(engine::EngineData& data, const std::string& move);
    void go(engine::EngineData& data, bool dont_play_move);
    void stop(engine::EngineData& data);
    void getparameters(const engine::EngineData& data);
    void getparameter(const engine::EngineData& data, const std::string& name);
    void setparameter(engine::EngineData& data, const std::string& name, const std::string& value);
    void quit(engine::EngineData& data);
}
