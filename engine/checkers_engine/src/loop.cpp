#include "loop.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cstring>
#include <cassert>

#include "commands.hpp"
#include "messages.hpp"
#include "error.hpp"

namespace loop {
    static std::string read_input() {
        // Doesn't return the new line

        std::string result;

        while (true) {
            char buffer[256] {};
            std::cin.getline(buffer, sizeof(buffer));

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

    static std::vector<std::string> tokenize_input(std::string&& input) {
        std::vector<std::string> tokens;

        char* token {std::strtok(input.data(), " \t")};

        while (token != nullptr) {
            tokens.emplace_back(token);
            token = std::strtok(nullptr, " \t");
        }

        return tokens;
    }

    static void execute_command(engine::Engine& engine, const std::vector<std::string>& input_tokens) {
        const auto& command_name {input_tokens.at(0)};

        // Commands may throw errors

        if (command_name == "INIT") {
            commands::init(engine, input_tokens);
        } else if (command_name == "NEWGAME") {
            commands::newgame(engine, input_tokens);
        } else if (command_name == "MOVE") {
            commands::move(engine, input_tokens);
        } else if (command_name == "GO") {
            commands::go(engine, input_tokens);
        } else if (command_name == "STOP") {
            commands::stop(engine, input_tokens);
        } else if (command_name == "GETPARAMETERS") {
            commands::getparameters(engine, input_tokens);
        } else if (command_name == "GETPARAMETER") {
            commands::getparameter(engine, input_tokens);
        } else if (command_name == "SETPARAMETER") {
            commands::setparameter(engine, input_tokens);
        } else if (command_name == "GETNAME") {
            commands::getname(engine, input_tokens);
        } else {
            throw error::InvalidCommand();
        }
    }

    int main_loop(engine::Engine& engine) {
        while (true) {
            auto input {read_input()};

            const std::vector<std::string> tokens {tokenize_input(std::move(input))};

            if (tokens.empty()) {
                continue;
            }

            // Handle QUIT separately
            if (tokens.at(0) == "QUIT") {
                commands::quit(engine, tokens);
                return 0;
            }

            // Ignore invalid commnads
            try {
                execute_command(engine, tokens);
            } catch (error::InvalidCommand) {
                continue;
            }
        }
    }
}
