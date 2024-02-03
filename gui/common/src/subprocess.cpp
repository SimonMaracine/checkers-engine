#include "common/subprocess.hpp"

#include <cstddef>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <sys/wait.h>

namespace subprocess {
    Subprocess::Subprocess(const std::string& file_path) {
        // const char* const cmd[] { file_path.c_str(), nullptr };

        // subprocess = new struct subprocess_s;
        // const int result {subprocess_create(cmd, subprocess_option_enable_async, subprocess)};

        // if (result != 0) {
        //     delete subprocess;
        //     throw 0;
        // }

        // output = subprocess_stdin(subprocess);

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
            close(fd_w[1u]);
            close(fd_r[0u]);

            if (dup2(STDIN_FILENO, fd_w[0u]) < 0) {
                std::exit(1);
            }

            if (dup2(STDOUT_FILENO, fd_r[1u]) < 0) {
                std::exit(1);
            }

            char* const argv[] { const_cast<char*>(file_path.c_str()), nullptr };

            if (execv(file_path.c_str(), argv) < 0) {
                std::exit(1);
            }

            // Execution stops in execv
        } else {
            // Parent writes to fd_w[1]
            // Parent reads from fd_r[0]

            input = fd_r[0u];
            output = fd_w[1u];
            child_pid = pid;
        }
    }

    Subprocess::~Subprocess() {
        // if (subprocess == nullptr) {
        //     return;
        // }

        // const int result {subprocess_destroy(subprocess)};

        // if (result != 0) {
        //     // TODO error
        // }

        // delete subprocess;

        if (child_pid < 0) {
            return;
        }

        wait_for();
    }

    bool Subprocess::read_from(std::string& data) const {
        // char buffer[512u] {};

        // const unsigned int bytes {subprocess_read_stdout(subprocess, buffer, 512u)};

        // if (bytes == 0u) {
        //     return true;
        // }

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
        // if (std::fprintf(output, "%s\n", data.c_str()) < 0) {
        //     return false;
        // }

        // return true;

        const ssize_t bytes {write(output, data.c_str(), data.size())};

        if (bytes < 0) {
            return false;
        } else if (bytes < data.size()) {
            return false;
        }

        return true;
    }

    bool Subprocess::wait_for() {
        // int process_return {};
        // const int result {subprocess_join(subprocess, &process_return)};

        // if (result != 0) {
        //     return std::nullopt;
        // }

        // return std::make_optional(process_return);

        if (waitpid(child_pid, nullptr, 0) < 0) {
            return false;
        }

        return true;
    }
}
