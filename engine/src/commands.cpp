#include "commands.hpp"

#include <string>
#include <optional>

namespace commands {
    bool try_init(engine::EngineData& data, const loop::InputTokens& input_tokens) {  // TODO use input tokens as arguments
        engine::init(data);

        return true;
    }

    bool try_newgame(engine::EngineData& data, const loop::InputTokens& input_tokens) {
        if (input_tokens.count == 2u) {
            engine::newgame(data, std::make_optional(input_tokens.tokens[1u]), std::nullopt);
        } else {
            engine::newgame(data, std::nullopt, std::nullopt);
        }

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
        if (input_tokens.count == 2u) {
            if (input_tokens.tokens[1u] == "dontplaymove") {
                engine::go(data, true);

                return true;
            }
        } else {
            engine::go(data, false);

            return true;
        }

        return false;
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
