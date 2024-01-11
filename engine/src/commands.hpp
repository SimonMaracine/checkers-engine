#pragma once

#include <functional>

#include "loop.hpp"
#include "engine.hpp"

namespace commands {
    using TryCommand = std::function<bool(engine::EngineData&, const loop::InputTokens&)>;

    bool try_init(engine::EngineData& data, const loop::InputTokens& input_tokens);
    bool try_newgame(engine::EngineData& data, const loop::InputTokens& input_tokens);
    bool try_move(engine::EngineData& data, const loop::InputTokens& input_tokens);
    bool try_go(engine::EngineData& data, const loop::InputTokens& input_tokens);
    bool try_setparameter(engine::EngineData& data, const loop::InputTokens& input_tokens);
    bool try_getparameter(engine::EngineData& data, const loop::InputTokens& input_tokens);
}
