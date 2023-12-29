#pragma once

#include <queue>
#include <string>
#include <optional>

class IoCommunication {
public:
    void run();

    std::optional<std::string> get_message();
    void put_message(const std::string& message);
private:
    std::queue<int> input_queue;
    std::queue<int> output_queue;
};
