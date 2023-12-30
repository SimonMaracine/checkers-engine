#include <csignal>
#include <cstdlib>
#include <utility>
#include <iostream>

#include "io.hpp"

volatile bool running {true};

int main() {
    const auto handler {[](int) {
        running = false;
    }};

    if (std::signal(SIGINT, handler) == SIG_ERR) {
        std::exit(1);
    }

    IoCommunication io;
    io.start();

    while (running) {
        const auto message {io.get_message()};

        if (!message) {
            continue;
        }

        io.put_message(*message);
    }

    io.stop();
}
