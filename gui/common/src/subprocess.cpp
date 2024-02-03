#include "common/subprocess.hpp"

#include <cstddef>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>

namespace subprocess {
    Subprocess::Subprocess(const std::string& file_path) {
        int fd_r[2u] {};
        if (pipe(fd_r) < 0) {
            throw 0;
        }

        int fd_w[2u] {};
        if (pipe(fd_w) < 0) {
            throw 0;
        }

        const pid_t pid {fork()};

        if (pid < 0) {
            throw 0;
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
            // Parent reads from fd_r[0]
            // Parent writes to fd_w[1]

            input = fd_r[0u];
            output = fd_w[1u];
            child_pid = pid;
        }
    }

    Subprocess::~Subprocess() {
        if (child_pid < 0) {
            return;
        }

        wait_for();
    }

    bool Subprocess::read_from(std::string& data) const {
        fd_set set;
        FD_ZERO(&set);

        timeval time;
        time.tv_sec = 0;
        time.tv_usec = 0;

        const int result {select(input + 1, &set, nullptr, nullptr, &time)};

        if (result < 0) {
            return false;
        } else if (result != 1) {
            return false;
        }

        char buffer[512u] {};

        const ssize_t bytes {read(input, buffer, 512u)};

        if (bytes < 0) {
            return false;
        }

        if (bytes == 0) {
            return true;
        }

        std::size_t bytes_consuming {0u};
        bool found_new_line {false};

        for (ssize_t i {0}; i < bytes; i++) {
            if (buffer[i] == '\n') {
                bytes_consuming = i;
                found_new_line = true;
                break;
            }
        }

        if (!found_new_line) {
            bytes_consuming = bytes;
        }

        char* final_buffer {nullptr};
        std::size_t final_bytes_consuming {};

        if (!buffered.empty()) {
            final_bytes_consuming = buffered.size() + bytes_consuming;
            final_buffer = new char[final_bytes_consuming];

            std::memcpy(final_buffer, buffered.data(), buffered.size());
            std::memcpy(final_buffer + buffered.size(), buffer, bytes_consuming);
        } else {
            final_bytes_consuming = bytes_consuming;
            final_buffer = new char[final_bytes_consuming];

            std::memcpy(final_buffer, buffer, bytes_consuming);
        }

        data = std::string(final_buffer, final_bytes_consuming);

        delete[] final_buffer;

        if (bytes_consuming < bytes) {
            buffered.resize(bytes - bytes_consuming);
            std::memcpy(buffered.data(), buffer + bytes_consuming, bytes - bytes_consuming);
        }

        return true;
    }

    bool Subprocess::write_to(const std::string& data) const {
        const ssize_t bytes {write(output, data.c_str(), data.size())};

        if (bytes < 0) {
            return false;
        } else if (bytes < data.size()) {
            return false;
        }

        return true;
    }

    bool Subprocess::wait_for() const {
        if (waitpid(child_pid, nullptr, 0) < 0) {
            return false;
        }

        return true;
    }
}
