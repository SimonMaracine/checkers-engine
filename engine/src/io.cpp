#include "io.hpp"

#include <iostream>
#include <utility>
#include <cassert>

void IoCommunication::start() {
    input_thread = std::thread([this]() { input_loop(); });
    output_thread = std::thread([this]() { output_loop(); });
}

void IoCommunication::stop() {
    running = false;

    input_thread.join();
    output_thread.join();
}

std::optional<std::string> IoCommunication::get_message() {
    if (input_queue.empty()) {
        return std::nullopt;
    }

    std::string message {input_queue.front()};
    input_queue.pop();

    return std::make_optional(message);
}

void IoCommunication::put_message(const std::string& message) {
    output_queue.push(message);
}

void IoCommunication::input_loop() {
    while (running) {
        char buffer[128] {};
        std::cin.getline(buffer, sizeof(buffer));

        if (std::cin.fail()) {
            std::cout << "fail\n";
            std::cin.clear();
            continue;
        }

        input_received(std::string(buffer));
    }
}

void IoCommunication::output_loop() {
    while (running) {
        if (output_queue.empty()) {
            continue;
        }

        std::string message {output_queue.front()};
        output_queue.pop();

        std::cout << message << '\n';
    }
}

void IoCommunication::input_received(std::string&& message) {
    input_queue.push(std::move(message));
}
