#pragma once

#include <string>
#include <cstdio>
#include <optional>

struct subprocess_s;

class Subprocess {
public:
    Subprocess() = default;
    Subprocess(const std::string& file_path);
    ~Subprocess();

    void send(const std::string& data) const;
    std::optional<std::string> receive() const;

    std::optional<int> join();
private:
    struct subprocess_s* subprocess {nullptr};
    std::FILE* input {nullptr};
    std::FILE* output {nullptr};
};
