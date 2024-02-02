#pragma once

#include <string>
#include <cstdio>
#include <optional>
#include <vector>

struct subprocess_s;

class Subprocess {
public:
    Subprocess() = default;
    Subprocess(const std::string& file_path);
    ~Subprocess();

    bool send(const std::string& data) const;
    bool receive(std::string& data) const;

    std::optional<int> join();
private:
    struct subprocess_s* subprocess {nullptr};
    std::FILE* output {nullptr};

    mutable struct {
        std::vector<char> buffered;
    } input;
};
