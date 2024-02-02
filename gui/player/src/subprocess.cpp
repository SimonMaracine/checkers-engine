#include "subprocess.hpp"

#include <subprocess.h>

Subprocess::Subprocess(const std::string& file_path) {
    const char* cmd[] { file_path.c_str(), nullptr };

    subprocess = new struct subprocess_s;
    const int result {subprocess_create(cmd, 0, subprocess)};

    if (result != 0) {
        return;
    }

    input = subprocess_stdin(subprocess);
    // std::fputs("Hello, world!", p_stdin);

    output = subprocess_stdout(subprocess);
    // char hello_world[32];
    // std::fgets(hello_world, 32, p_stdout);
}

Subprocess::~Subprocess() {
    const int result {subprocess_destroy(subprocess)};

    if (result != 0) {

    }

    delete subprocess;
}

int Subprocess::join() {
    int process_return {};
    const int result {subprocess_join(subprocess, &process_return)};

    if (result != 0) {

    }

    return process_return;
}
