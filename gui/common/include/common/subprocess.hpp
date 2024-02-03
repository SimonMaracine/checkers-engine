#pragma once

#include <string>
#include <cstdio>
#include <optional>
#include <vector>

namespace subprocess {
    class Subprocess {
    public:
        Subprocess() = default;
        Subprocess(const std::string& file_path);
        ~Subprocess();

        bool read(std::string& data) const;
        bool write(const std::string& data) const;

        std::optional<int> join();
    private:

        std::FILE* output {nullptr};

        mutable struct {
            std::vector<char> buffered;
        } input;
    };
}
