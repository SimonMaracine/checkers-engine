#pragma once

#include <functional>
#include <vector>
#include <string>

#include "engine.hpp"

namespace commands {
    void init(engine::Engine& engine, const std::vector<std::string>&);
    void newgame(engine::Engine& engine, const std::vector<std::string>& tokens);
    void move(engine::Engine& engine, const std::vector<std::string>& tokens);
    void go(engine::Engine& engine, const std::vector<std::string>& tokens);
    void stop(engine::Engine& engine, const std::vector<std::string>&);
    void getparameters(engine::Engine& engine, const std::vector<std::string>&);
    void setparameter(engine::Engine& engine, const std::vector<std::string>& tokens);
    void getparameter(engine::Engine& engine, const std::vector<std::string>& tokens);
    void quit(engine::Engine& engine, const std::vector<std::string>&);
    void getname(engine::Engine& engine, const std::vector<std::string>&);
}
