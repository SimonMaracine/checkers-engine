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

// https://en.cppreference.com/w/cpp/io/basic_istream/getline

namespace loop {
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
        } else if (command_name == "BOARD") {
            commands::board(engine, input_tokens);
        } else {
            throw error::InvalidCommand();
        }
    }

    std::string read_input() {
        // Doesn't return the new line

        std::string result;

        while (true) {
            char buffer[256] {};
            std::cin.getline(buffer, sizeof(buffer));

            result += buffer;

            // Just go on
            if (std::cin.eof()) {
                std::cin.clear();
                throw error::Fatal();
                // continue;
            }

            // An error occurred...
            if (std::cin.bad()) {
                std::cin.clear();
                return result;
            }

            // Extracted 256 characters without a new line
            if (std::cin.fail()) {
                std::cin.clear();
                continue;
            }

            return result;
        }
    }

    int main_loop(engine::Engine& engine, std::string(*read_input)()) {
        while (true) {
            std::vector<std::string> tokens;

            try {
                tokens = tokenize_input(read_input());
            } catch (error::Fatal) {
                commands::quit(engine, tokens);
                return 1;
            }

            if (tokens.empty()) {
                continue;
            }

            // Handle QUIT separately
            if (tokens.at(0) == "QUIT") {
                commands::quit(engine, tokens);
                return 0;
            }

            // Ignore invalid commands
            try {
                execute_command(engine, tokens);
            } catch (error::InvalidCommand) {
                continue;
            } catch (error::Fatal) {
                commands::quit(engine, tokens);
                return 1;
            }
        }
    }
}
