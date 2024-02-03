#pragma once

#include <string>
#include <vector>

namespace subprocess {
    class Subprocess {
    public:
        Subprocess() = default;
        Subprocess(const std::string& file_path);
        ~Subprocess();

        // FIXME avoid copy/move problems
        Subprocess(const Subprocess&) = default;
        Subprocess& operator=(const Subprocess&) = default;
        Subprocess(Subprocess&&) = default;
        Subprocess& operator=(Subprocess&&) = default;

        bool read_from(std::string& data) const;
        bool write_to(const std::string& data) const;
        bool wait_for();
    private:
        int input {};  // Read from
        int output {};  // Write to
        int child_pid {-1};

        mutable std::vector<char> buffered;
    };
}
