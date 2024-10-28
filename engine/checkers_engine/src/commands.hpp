#pragma once

#include <functional>
#include <vector>
#include <string>

#include "engine.hpp"

namespace commands {
    void init(engine::EngineData& data, const std::vector<std::string>&);
    void newgame(engine::EngineData& data, const std::vector<std::string>& tokens);
    void move(engine::EngineData& data, const std::vector<std::string>& tokens);
    void go(engine::EngineData& data, const std::vector<std::string>& tokens);
    void stop(engine::EngineData& data, const std::vector<std::string>&);
    void getparameters(engine::EngineData& data, const std::vector<std::string>&);
    void setparameter(engine::EngineData& data, const std::vector<std::string>& tokens);
    void getparameter(engine::EngineData& data, const std::vector<std::string>& tokens);
    void quit(engine::EngineData& data, const std::vector<std::string>&);
}
