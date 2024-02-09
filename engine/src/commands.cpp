#include "commands.hpp"

#include <string>
#include <optional>

// TODO refactor how messages are processed and how error is handled (or not handled)

namespace commands {
    bool try_init(engine::EngineData& data, const loop::InputTokens&) {
        engine::init(data);

        return true;
    }

    bool try_newgame(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        if (input_tokens.find(1u)) {
            engine::newgame(data, std::make_optional(input_tokens[1u]), std::nullopt);
        } else {
            engine::newgame(data, std::nullopt, std::nullopt);
        }

        return true;
    }

    bool try_move(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        if (!input_tokens.find(1u)) {
            return false;
        }

        engine::move(data, input_tokens[1u]);

        return true;
    }

    bool try_go(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        if (input_tokens.find(1u)) {
            if (input_tokens[1u] == "dontplaymove") {
                engine::go(data, true);

                return true;
            }
        } else {
            engine::go(data, false);

            return true;
        }

        return false;
    }

    bool try_getparameters(engine::EngineData& data, const loop::InputTokens&) {
        engine::getparameters(data);

        return true;
    }

    bool try_setparameter(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        if (!input_tokens.find(1u) || !input_tokens.find(2u)) {
            return false;
        }

        engine::setparameter(data, input_tokens[1u], input_tokens[2u]);

        return true;
    }

    bool try_getparameter(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        if (!input_tokens.find(1u)) {
            return false;
        }

        engine::getparameter(data, input_tokens[1u]);

        return true;
    }
}
