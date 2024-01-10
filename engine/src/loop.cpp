#include "loop.hpp"

#include <iostream>
#include <string>
#include <array>
#include <cstddef>
#include <cstring>

struct InputTokens {
    static constexpr std::size_t MAX {8u};

    std::array<std::string, MAX> tokens {};
    std::size_t count {};
};

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

static void send_message_error() {
    std::cout << "ERRORCOMMAND\n";
}

void main_loop(EngineData& data) {
    while (true) {
        char input[128] {};
        std::cin.getline(input, 128);

        const InputTokens input_tokens {tokenize_input(input)};

        if (input_tokens.count == 0) {
            continue;
        }

        const auto& command {input_tokens.tokens[0]};

        if (command == "INIT") {

        } else if (command == "NEWGAME") {

        } else if (command == "MOVE") {

        } else if (command == "GO") {

        } else if (command == "SETPARAMETER") {

        } else if (command == "GETPARAMETER") {

        } else if (command == "QUIT") {
            break;
        } else {
            send_message_error();
        }
    }
}
