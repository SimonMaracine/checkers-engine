#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <utility>
#include <cassert>

namespace input_tokens {
    class InputTokens {
    public:
        explicit InputTokens(std::vector<std::string>&& tokens)
            : tokens(std::move(tokens)) {}

        std::size_t size() const {
            return tokens.size();
        }

        bool empty() const {
            return tokens.empty();
        }

        bool find(std::size_t index) const {
            return index < tokens.size();
        }

        const std::string& operator[](std::size_t index) const {
            assert(index < tokens.size());

            return tokens[index];
        }
    private:
        std::vector<std::string> tokens;
    };
}
