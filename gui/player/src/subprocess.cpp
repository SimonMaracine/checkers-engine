#include "subprocess.hpp"

#include <subprocess.h>

Subprocess::Subprocess(const std::string& file_path) {
    const char* const cmd[] { file_path.c_str(), nullptr };

    subprocess = new struct subprocess_s;
    const int result {subprocess_create(cmd, 0, subprocess)};

    if (result != 0) {
        return;
    }

    input = subprocess_stdin(subprocess);
    output = subprocess_stdout(subprocess);
}

Subprocess::~Subprocess() {
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

void Subprocess::send(const std::string& data) const {
    if (std::fprintf(output, "%s\n", data.c_str()) < 0) {
        // TODO error
    }
}

std::optional<std::string> Subprocess::receive() const {
    char buffer[512u] {};

    if (std::fgets(buffer, 512, input) == nullptr) {
        return std::nullopt;
    }

    return std::make_optional(buffer);
}
