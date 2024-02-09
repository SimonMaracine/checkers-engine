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
            std::function<SearchResult()> search;
            std::condition_variable cv;
            std::mutex mutex;
            std::unordered_map<std::string, Param> parameters;
            bool running {false};
       } minimax;

        struct {
            game::Position position;  // Internal position
            std::vector<game::Position> previous_positions;
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
    void getparameters(engine::EngineData& data);
    void getparameter(engine::EngineData& data, const std::string& name);
    void setparameter(engine::EngineData& data, const std::string& name, const std::string& value);
    void quit(engine::EngineData& data);

    // TODO maybe initialization, finalization
}
