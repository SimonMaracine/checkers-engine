#include "commands.hpp"

#include <string>
#include <optional>
#include <cstddef>
#include <vector>

namespace commands {
    static std::optional<std::vector<std::string>> parse_setup_moves(const input_tokens::InputTokens& tokens) {
        std::size_t index {2u};
        std::vector<std::string> setup_moves;

        while (tokens.find(index)) {
            setup_moves.push_back(tokens[index]);

            index++;
        }

        if (setup_moves.empty()) {
            return std::nullopt;
        } else {
            return std::make_optional(setup_moves);
        }
    }

    void init(engine::EngineData& data, const input_tokens::InputTokens&) {
        engine::init(data);
    }

    void newgame(engine::EngineData& data, const input_tokens::InputTokens& tokens) {
        const auto setup_moves {parse_setup_moves(tokens)};

        if (tokens.find(1u)) {
            engine::newgame(data, std::make_optional(tokens[1u]), setup_moves);
        } else {
            engine::newgame(data, std::nullopt, setup_moves);
        }
    }

    void move(engine::EngineData& data, const input_tokens::InputTokens& tokens) {
        if (!tokens.find(1u)) {
            return;
        }

        engine::move(data, tokens[1u]);
    }

    void go(engine::EngineData& data, const input_tokens::InputTokens& tokens) {
        if (tokens.find(1u)) {
            if (tokens[1u] == "dontplaymove") {
                engine::go(data, true);
            }
        } else {
            engine::go(data, false);
        }
    }

    void stop(engine::EngineData& data, const input_tokens::InputTokens&) {
        engine::stop(data);
    }

    void getparameters(engine::EngineData& data, const input_tokens::InputTokens&) {
        engine::getparameters(data);
    }

    void setparameter(engine::EngineData& data, const input_tokens::InputTokens& tokens) {
        if (!tokens.find(1u) || !tokens.find(2u)) {
            return;
        }

        engine::setparameter(data, tokens[1u], tokens[2u]);
    }

    void getparameter(engine::EngineData& data, const input_tokens::InputTokens& tokens) {
        if (!tokens.find(1u)) {
            return;
        }

        engine::getparameter(data, tokens[1u]);
    }

    void quit(engine::EngineData& data, const input_tokens::InputTokens&) {
        engine::quit(data);
    }
}
