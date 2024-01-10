#include "commands.hpp"

#include <string>

namespace commands {
    static void init(engine::EngineData& data) {

    }

    static void newgame(engine::EngineData& data) {

    }

    static void move(engine::EngineData& data, const std::string& move) {

    }

    static void go(engine::EngineData& data, bool dont_play_move) {

    }

    static void setparameter(engine::EngineData& data, const std::string& name, const std::string& value) {

    }

    static void getparameter(engine::EngineData& data, const std::string& name) {

    }

    bool try_init(const loop::InputTokens& input_tokens, engine::EngineData& data) {
        init(data);

        return true;
    }

    bool try_newgame(const loop::InputTokens& input_tokens, engine::EngineData& data) {
        newgame(data);

        return true;
    }

    bool try_move(const loop::InputTokens& input_tokens, engine::EngineData& data) {
        move(data, "");

        return true;
    }

    bool try_go(const loop::InputTokens& input_tokens, engine::EngineData& data) {
        go(data, true);

        return true;
    }

    bool try_setparameter(const loop::InputTokens& input_tokens, engine::EngineData& data) {
        setparameter(data, "", "");

        return true;
    }

    bool try_getparameter(const loop::InputTokens& input_tokens, engine::EngineData& data) {
        getparameter(data, "");

        return true;
    }
}
