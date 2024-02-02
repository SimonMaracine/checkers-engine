#pragma once

#include <string>
#include <cstdio>

struct subprocess_s;

class Subprocess {
public:
    Subprocess(const std::string& file_path);
    ~Subprocess();

    int join();
private:
    struct subprocess_s* subprocess;
    std::FILE* input {nullptr};
    std::FILE* output {nullptr};
};
