#pragma once

#include <functional>

#include "engine.hpp"
#include "input_tokens.hpp"

namespace commands {
    using Command = std::function<void(engine::EngineData&, const input_tokens::InputTokens&)>;

    void init(engine::EngineData& data, const input_tokens::InputTokens&);
    void newgame(engine::EngineData& data, const input_tokens::InputTokens& tokens);
    void move(engine::EngineData& data, const input_tokens::InputTokens& tokens);
    void go(engine::EngineData& data, const input_tokens::InputTokens& tokens);
    void stop(engine::EngineData& data, const input_tokens::InputTokens&);
    void getparameters(engine::EngineData& data, const input_tokens::InputTokens&);
    void setparameter(engine::EngineData& data, const input_tokens::InputTokens& tokens);
    void getparameter(engine::EngineData& data, const input_tokens::InputTokens& tokens);
    void quit(engine::EngineData& data, const input_tokens::InputTokens&);
}
