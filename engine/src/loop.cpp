#include "loop.hpp"

#include <iostream>
#include <cstring>
#include <unordered_map>

#include "commands.hpp"
#include "messages.hpp"

namespace loop {
    static InputTokens tokenize_input(const char* input) {
        InputTokens result;

        std::string mutable_buffer {input};

        char* token {std::strtok(mutable_buffer.data(), " \t")};  // TODO other whitespace characters?

        while (token != nullptr) {
            if (result.count == InputTokens::MAX) {
                break;
            }

            result.tokens[result.count++] = token;

            token = std::strtok(nullptr, " \t");
        }

        return result;
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

        const auto& command_name {input_tokens.tokens[0]};

        if (COMMANDS.find(command_name) == COMMANDS.cend()) {
            return false;
        }

        // Call the command
        if (!COMMANDS.at(command_name)(data, input_tokens)) {
            return false;
        }

        return true;
    }

    void main_loop(engine::EngineData& data) {
        while (true) {
            char input[128] {};
            std::cin.getline(input, 128);

            const InputTokens input_tokens {tokenize_input(input)};

            if (input_tokens.count == 0) {
                continue;
            }

            // Handle QUIT separately
            if (input_tokens.tokens[0] == "QUIT") {
                break;
            }

            if (!execute_command(data, input_tokens)) {
                messages::errorcommand();
            }
        }
    }
}
