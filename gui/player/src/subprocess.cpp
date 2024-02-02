#include "subprocess.hpp"

#include <iterator>
#include <utility>
#include <cstddef>
#include <cstring>

#include <subprocess.h>

Subprocess::Subprocess(const std::string& file_path) {
    const char* const cmd[] { file_path.c_str(), nullptr };

    subprocess = new struct subprocess_s;
    const int result {subprocess_create(cmd, subprocess_option_enable_async, subprocess)};

    if (result != 0) {
        delete subprocess;
        throw 0;
    }

    output = subprocess_stdin(subprocess);
}

Subprocess::~Subprocess() {
    if (subprocess == nullptr) {
        return;
    }

    const int result {subprocess_destroy(subprocess)};

    if (result != 0) {
        // TODO error
    }

    delete subprocess;
}

std::optional<int> Subprocess::join() {
    int process_return {};
    const int result {subprocess_join(subprocess, &process_return)};

    if (result != 0) {
        return std::nullopt;
    }

    return std::make_optional(process_return);
}

bool Subprocess::send(const std::string& data) const {
    if (std::fprintf(output, "%s\n", data.c_str()) < 0) {
        return false;
    }

    return true;
}

bool Subprocess::receive(std::string& data) const {
    char buffer[512u] {};

    const unsigned int bytes {subprocess_read_stdout(subprocess, buffer, 512u)};

    if (bytes == 0u) {
        return true;
    }

    std::size_t bytes_consuming {0u};
    bool found_new_line {false};

    for (unsigned int i {0u}; i < bytes; i++) {
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

    if (!input.buffered.empty()) {
        final_bytes_consuming = input.buffered.size() + bytes_consuming;
        final_buffer = new char[final_bytes_consuming];

        std::memcpy(final_buffer, input.buffered.data(), input.buffered.size());
        std::memcpy(final_buffer + input.buffered.size(), buffer, bytes_consuming);
    } else {
        final_bytes_consuming = bytes_consuming;
        final_buffer = new char[final_bytes_consuming];

        std::memcpy(final_buffer, buffer, bytes_consuming);
    }

    data = std::string(final_buffer, final_bytes_consuming);

    delete[] final_buffer;

    if (bytes_consuming < bytes) {
        input.buffered.resize(bytes - bytes_consuming);
        std::memcpy(input.buffered.data(), buffer + bytes_consuming, bytes - bytes_consuming);
    }

    return true;
}
