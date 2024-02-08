#include "loop.hpp"

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <cassert>

#include "commands.hpp"
#include "messages.hpp"
#include "error.hpp"

namespace loop {
    static InputTokens tokenize_input(const char* input) {
        std::vector<std::string> tokens;

        std::string mutable_buffer {input};

        char* token {std::strtok(mutable_buffer.data(), " \t")};  // TODO other whitespace characters?

        while (token != nullptr) {
            if (tokens.size() == 8u) {
                break;
            }

            tokens.emplace_back(token);

            token = std::strtok(nullptr, " \t");
        }

        return InputTokens(std::move(tokens));
    }

    static bool execute_command(engine::EngineData& data, const InputTokens& input_tokens) {
        static const std::unordered_map<std::string, commands::TryCommand> COMMANDS {
            { "INIT", commands::try_init },
            { "NEWGAME", commands::try_newgame },
            { "MOVE", commands::try_move },
            { "GO", commands::try_go },
            { "SETPARAMETER", commands::try_setparameter },
            { "GETPARAMETER", commands::try_getparameter }
        };

        const auto command_name {input_tokens[0u]};

        if (COMMANDS.find(command_name) == COMMANDS.cend()) {
            return false;
        }

        // Call the command; may throw errors
        if (!COMMANDS.at(command_name)(data, input_tokens)) {
            return false;
        }

        return true;
    }

    bool InputTokens::find(std::size_t index) const {
        return index < tokens.size();
    }

    const std::string& InputTokens::operator[](std::size_t index) const {
        assert(index < tokens.size());

        return tokens[index];
    }

    int main_loop(engine::EngineData& data) {
        while (true) {
            char input[128u] {};
            std::cin.getline(input, 128);

            const InputTokens input_tokens {tokenize_input(input)};

            if (input_tokens.size() == 0u) {
                continue;
            }

            // Handle QUIT separately
            if (input_tokens[0u] == "QUIT") {
                engine::quit(data);
                break;
            }

            try {
                if (!execute_command(data, input_tokens)) {
                    messages::warning();
                }
            } catch (error::Error) {
                return 1;
            }
        }

        return 0;
    }
}
