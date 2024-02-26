#include "common/subprocess.hpp"

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <utility>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>

namespace subprocess {
    template<std::size_t Size>
    static bool newline_in_buffer(const char* buffer, std::size_t& bytes_up_to_newline) {
        for (std::size_t i {0u}; i < Size; i++) {
            if (buffer[i] == '\n') {
                bytes_up_to_newline = i + 1u;  // Including newline
                return true;
            }
        }

        bytes_up_to_newline = 0u;
        return false;
    }

    static bool newline_in_buffer(const std::string& buffer, std::size_t& bytes_up_to_newline) {
        for (std::size_t i {0u}; i < buffer.size(); i++) {
            if (buffer[i] == '\n') {
                bytes_up_to_newline = i + 1u;  // Including newline
                return true;
            }
        }

        bytes_up_to_newline = 0u;
        return false;
    }

    Subprocess::Subprocess(const std::string& file_path) {
        int fd_r[2u] {};
        if (pipe(fd_r) < 0) {
            throw Error();
        }

        int fd_w[2u] {};
        if (pipe(fd_w) < 0) {
            throw Error();
        }

        const pid_t pid {fork()};

        if (pid < 0) {
            throw Error();
        } else if (pid == 0) {
            close(fd_r[0u]);
            close(fd_w[1u]);

            if (dup2(fd_r[1u], STDOUT_FILENO) < 0) {
                std::exit(1);
            }

            if (dup2(fd_w[0u], STDIN_FILENO) < 0) {
                std::exit(1);
            }

            char* const argv[] { const_cast<char*>(file_path.c_str()), nullptr };

            if (execv(file_path.c_str(), argv) < 0) {
                std::exit(1);
            }

            // Child execution stops in execv
        } else {
            close(fd_r[1u]);
            close(fd_w[0u]);

            // Parent reads from fd_r[0]
            // Parent writes to fd_w[1]

            input = fd_r[0u];
            output = fd_w[1u];
            child_pid = pid;
        }
    }

    Subprocess::~Subprocess() {
        assert(child_pid < 0);
    }

    Subprocess::Subprocess(Subprocess&& other) noexcept {
        input = other.input;
        output = other.output;
        child_pid = other.child_pid;
        buffered = std::move(other.buffered);

        other.child_pid = -1;
    }

    Subprocess& Subprocess::operator=(Subprocess&& other) noexcept {
        assert(child_pid < 0);

        input = other.input;
        output = other.output;
        child_pid = other.child_pid;
        buffered = std::move(other.buffered);

        other.child_pid = -1;

        return *this;
    }

    bool Subprocess::read_from(std::string& data) const {
        {
            std::size_t bytes_up_to_newline {};

            if (newline_in_buffer(buffered, bytes_up_to_newline)) {
                const std::string current = std::string(buffered, 0u, bytes_up_to_newline);

                buffered = std::string(buffered, bytes_up_to_newline, static_cast<std::size_t>(buffered.size()));
                data = current;

                return true;
            }
        }

        fd_set set;
        FD_ZERO(&set);
        FD_SET(input, &set);

        timeval time;
        time.tv_sec = 0;
        time.tv_usec = 0;

        const int result {select(input + 1, &set, nullptr, nullptr, &time)};

        if (result < 0) {
            return false;
        } else if (result != 1) {
            return false;
        }

        // 1.  Read a bunch of bytes
        // 2.  Scan for newline in buffer
        // 3a. If a newline is found, concatenate the buffer up to the newline with the contents of the buffered buffer and return it as the result
        // 3b. Save the rest of the extracted characters (from newline + 1 up to the end) into the buffered buffer
        // 4.  If a newline is not found, save the buffer and goto #1

        static constexpr std::size_t CHUNK {256u};

        std::string current;

        while (true) {
            char buffer[CHUNK] {};
            const ssize_t bytes {read(input, buffer, CHUNK)};

            if (bytes < 0) {
                return false;
            }

            if (bytes == 0) {
                return false;
            }

            std::size_t bytes_up_to_newline {};

            if (newline_in_buffer<CHUNK>(buffer, bytes_up_to_newline)) {
                current += std::string(buffer, bytes_up_to_newline);

                data = (
                    std::exchange(buffered, std::string(buffer, bytes_up_to_newline, static_cast<std::size_t>(bytes)))
                    + current
                );

                return true;
            } else {
                current += buffer;
            }
        }
    }

    bool Subprocess::write_to(const std::string& data) const {
        const ssize_t bytes {write(output, data.c_str(), data.size())};

        if (bytes < 0) {
            return false;
        } else if (bytes < static_cast<ssize_t>(data.size())) {
            return false;
        }

        return true;
    }

    bool Subprocess::wait_for() {
        if (waitpid(std::exchange(child_pid, -1), nullptr, 0) < 0) {
            return false;
        }

        return true;
    }
}
