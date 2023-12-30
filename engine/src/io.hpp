#pragma once

#include <queue>
#include <string>
#include <optional>
#include <thread>

class IoCommunication {
public:
    void start();
    void stop();

    std::optional<std::string> get_message();
    void put_message(const std::string& message);
private:
    void input_loop();
    void output_loop();
    void input_received(std::string&& message);

    std::queue<std::string> input_queue;
    std::queue<std::string> output_queue;

    std::thread input_thread;
    std::thread output_thread;

    bool running {true};

    // TODO mutex
};
