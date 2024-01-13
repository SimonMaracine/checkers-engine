#include "engine.hpp"

namespace engine {
    static void reset(EngineData& data) {
        game::set_position(data.game.position.position, "B:B1,2,3,4,5,6,7,8,9,10,11,12:W21,22,23,24,25,26,27,28,29,30,31,32");

        data.game.position.plies = 0u;
        data.game.position.plies_without_advancement = 0u;

        data.game.previous_positions.clear();
        data.game.moves_played.clear();
    }

    void init(engine::EngineData& data) {
        reset(data);
    }

    void newgame(engine::EngineData& data) {
        reset(data);
    }

    void move(engine::EngineData& data, const std::string& move) {
        game::make_move(data.game.position, move);
    }

    void go(engine::EngineData& data, bool dont_play_move) {

    }

    void setparameter(engine::EngineData& data, const std::string& name, const std::string& value) {

    }

    void getparameter(engine::EngineData& data, const std::string& name) {

    }
}
