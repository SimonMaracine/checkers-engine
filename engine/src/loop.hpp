#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <utility>

#include "engine.hpp"

namespace loop {
    class InputTokens {
    public:
        InputTokens(std::vector<std::string>&& tokens)
            : tokens(std::move(tokens)) {}

        std::size_t size() const { return tokens.size(); }

        bool find(std::size_t index) const;
        const std::string& operator[](std::size_t index) const;
    private:
        std::vector<std::string> tokens;
    };

    int main_loop(engine::EngineData& data);
}
