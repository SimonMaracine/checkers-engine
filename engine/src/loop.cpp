#include "loop.hpp"

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <cassert>

#include "commands.hpp"
#include "messages.hpp"
#include "error.hpp"

namespace loop {
    // Doesn't return the new line
    static std::string read_input() {
        static constexpr std::size_t CHUNK {256u};

        std::string result;

        while (true) {
            char buffer[CHUNK] {};
            std::cin.getline(buffer, static_cast<std::streamsize>(CHUNK));

            result += buffer;

            if (std::cin.bad()) {
                std::cin.clear();

                return result;
            }

            if (std::cin.good()) {
                return result;
            } else {
                std::cin.clear();
            }
        }
    }

    static InputTokens tokenize_input(std::string&& input) {
        std::vector<std::string> tokens;

        char* token {std::strtok(input.data(), " \t")};  // TODO other whitespace characters?

        while (token != nullptr) {
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
            { "STOP", commands::try_stop },
            { "GETPARAMETERS", commands::try_getparameters },
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
            auto input {read_input()};

            const InputTokens input_tokens {tokenize_input(std::move(input))};

            if (input_tokens.empty()) {
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
