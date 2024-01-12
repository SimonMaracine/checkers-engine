#include "commands.hpp"

#include <string>

#include "game.hpp"

namespace commands {
    namespace core {
        static void init(engine::EngineData& data) {
            game::reset_position(data.position);
        }

        static void newgame(engine::EngineData& data) {
            game::reset_position(data.position);
        }

        static void move(engine::EngineData& data, const std::string& move) {

        }

        static void go(engine::EngineData& data, bool dont_play_move) {

        }

        static void setparameter(engine::EngineData& data, const std::string& name, const std::string& value) {

        }

        static void getparameter(engine::EngineData& data, const std::string& name) {

        }
    }

    bool try_init(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        core::init(data);

        return true;
    }

    bool try_newgame(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        core::newgame(data);

        return true;
    }

    bool try_move(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        core::move(data, "");

        return true;
    }

    bool try_go(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        core::go(data, true);

        return true;
    }

    bool try_setparameter(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        core::setparameter(data, "", "");

        return true;
    }

    bool try_getparameter(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        core::getparameter(data, "");

        return true;
    }
}
