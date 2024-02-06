#pragma once

#include <string>
#include <vector>

namespace subprocess {
    class Subprocess {
    public:
        Subprocess() = default;
        Subprocess(const std::string& file_path);
        ~Subprocess() = default;

        Subprocess(const Subprocess&) = delete;
        Subprocess& operator=(const Subprocess&) = delete;
        Subprocess(Subprocess&&) noexcept = default;
        Subprocess& operator=(Subprocess&&) noexcept = default;

        bool read_from(std::string& data) const;
        bool write_to(const std::string& data) const;
        bool wait_for() const;
    private:
        int input {};  // Read from
        int output {};  // Write to
        int child_pid {-1};

        mutable std::vector<char> buffered;
    };
}
