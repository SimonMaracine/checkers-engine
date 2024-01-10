#pragma once

#include <functional>

#include "loop.hpp"
#include "engine.hpp"

namespace commands {
    using TryCommand = std::function<bool(const loop::InputTokens&, engine::EngineData& data)>;

    bool try_init(const loop::InputTokens& input_tokens, engine::EngineData& data);
    bool try_newgame(const loop::InputTokens& input_tokens, engine::EngineData& data);
    bool try_move(const loop::InputTokens& input_tokens, engine::EngineData& data);
    bool try_go(const loop::InputTokens& input_tokens, engine::EngineData& data);
    bool try_setparameter(const loop::InputTokens& input_tokens, engine::EngineData& data);
    bool try_getparameter(const loop::InputTokens& input_tokens, engine::EngineData& data);
}
