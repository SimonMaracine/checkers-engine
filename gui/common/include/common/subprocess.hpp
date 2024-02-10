#pragma once

#include <string>

namespace subprocess {
    class Subprocess {
    public:
        Subprocess() = default;
        Subprocess(const std::string& file_path);
        ~Subprocess();

        Subprocess(const Subprocess&) = delete;
        Subprocess& operator=(const Subprocess&) = delete;
        Subprocess(Subprocess&& other) noexcept;
        Subprocess& operator=(Subprocess&& other) noexcept;

        bool read_from(std::string& data) const;
        bool write_to(const std::string& data) const;
        bool wait_for();  // Sets PID to -1
    private:
        int input {};  // Read from
        int output {};  // Write to
        int child_pid {-1};

        mutable std::string buffered;
    };
}
