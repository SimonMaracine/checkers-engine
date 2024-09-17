#include "loop.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cstddef>
#include <utility>
#include <cstring>
#include <unordered_map>
#include <cassert>

#include "commands.hpp"
#include "messages.hpp"
#include "input_tokens.hpp"
#include "error.hpp"

namespace loop {
    // Doesn't return the new line
    static std::string read_input() {
        static constexpr std::size_t CHUNK {256};

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

    static input_tokens::InputTokens tokenize_input(std::string&& input) {
        std::vector<std::string> tokens;

        char* token {std::strtok(input.data(), " \t")};

        while (token != nullptr) {
            tokens.emplace_back(token);

            token = std::strtok(nullptr, " \t");
        }

        return input_tokens::InputTokens(std::move(tokens));
    }

    static void execute_command(engine::EngineData& data, const input_tokens::InputTokens& input_tokens) {
        static const std::unordered_map<std::string, commands::Command> COMMANDS {
            { "INIT", commands::init },
            { "NEWGAME", commands::newgame },
            { "MOVE", commands::move },
            { "GO", commands::go },
            { "STOP", commands::stop },
            { "GETPARAMETERS", commands::getparameters },
            { "SETPARAMETER", commands::setparameter },
            { "GETPARAMETER", commands::getparameter }
        };

        const auto command_name {input_tokens[0]};

        if (COMMANDS.find(command_name) == COMMANDS.cend()) {
            return;
        }

        // Call the command; may throw errors
        COMMANDS.at(command_name)(data, input_tokens);
    }

    int main_loop(engine::EngineData& data) {
        while (true) {
            auto input {read_input()};

            const input_tokens::InputTokens tokens {tokenize_input(std::move(input))};

            if (tokens.empty()) {
                continue;
            }

            // Handle QUIT separately
            if (tokens[0] == "QUIT") {
                commands::quit(data, tokens);
                break;
            }

            try {
                execute_command(data, tokens);
            } catch (error::Error) {
                return 1;
            }
        }

        return 0;
    }
}
