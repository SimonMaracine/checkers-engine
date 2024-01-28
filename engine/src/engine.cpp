#include "engine.hpp"

#include <cassert>

#include "moves.hpp"

namespace engine {
    static void reset(EngineData& data) {
        game::set_position(data.game.position.position, "B:B1,2,3,4,5,6,7,8,9,10,11,12:W21,22,23,24,25,26,27,28,29,30,31,32");

        data.game.position.plies = 0u;
        data.game.position.plies_without_advancement = 0u;

        data.game.previous_positions.clear();
        data.game.moves_played.clear();
    }

    static void wait_sleeping(EngineData& data) {
        std::unique_lock<std::mutex> lock {data.minimax.mutex};
        data.minimax.cv.wait(lock, [&data]() { return static_cast<bool>(data.minimax.search); });
    }

    void init(engine::EngineData& data) {
        assert(!data.minimax.running);

        data.minimax.running = true;

        data.minimax.thread = std::thread([&data]() {
            while (true) {
                wait_sleeping(data);

                if (!data.minimax.running) {
                    break;
                }

                const game::Move best_move {data.minimax.search()};

                moves::play_move(data.game.position, best_move);
                // TODO store move and store position

                // Reset the function as a signal for the cv
                data.minimax.search = {};
            }
        });

        reset(data);
    }

    void newgame(engine::EngineData& data) {
        reset(data);
    }

    void move(engine::EngineData& data, const std::string& move) {
        game::make_move(data.game.position, move);
        // TODO store move and store position
    }

    void go(engine::EngineData& data, bool dont_play_move) {
        // TODO set search function and notify
    }

    void setparameter(engine::EngineData& data, const std::string& name, const std::string& value) {

    }

    void getparameter(engine::EngineData& data, const std::string& name) {

    }

    void quit(engine::EngineData& data) {
        assert(data.minimax.running);
        assert(!data.minimax.search);

        // Set dummy work to wake up the thread from sleeping
        data.minimax.search = []() -> game::Move { return {}; };
        data.minimax.running = false;

        data.minimax.cv.notify_one();
        data.minimax.thread.join();
    }
}
