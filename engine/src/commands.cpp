#include "commands.hpp"

#include <string>

namespace commands {
    bool try_init(engine::EngineData& data, const loop::InputTokens& input_tokens) {  // TODO use input tokens as arguments
        engine::init(data);

        return true;
    }

    bool try_newgame(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        engine::newgame(data);

        return true;
    }

    bool try_move(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        if (input_tokens.count != 2u) {
            return false;
        }

        engine::move(data, input_tokens.tokens[1u]);

        return true;
    }

    bool try_go(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        engine::go(data, true);

        return true;
    }

    bool try_setparameter(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        engine::setparameter(data, "", "");

        return true;
    }

    bool try_getparameter(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        engine::getparameter(data, "");

        return true;
    }
}
