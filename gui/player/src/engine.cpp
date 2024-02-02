#include "engine.hpp"

#include <cassert>

void EngineReader::Notify() {
    assert(process != nullptr);
    assert(callback);

    std::string message;

    if (!process->receive(message)) {
        return;
    }

    callback(message);
}

void Engine::start(const std::string& file_path, const ReadCallback& callback) {
    try {
        process = std::make_unique<Subprocess>(file_path);
    } catch (int) {
        // TODO error
        throw;
    }

    if (!process->send("INIT")) {
        // TODO error
    }

    reader = std::make_unique<EngineReader>(process.get(), callback);

    for (unsigned int i {0u}; i < 5u; i++) {
        if (reader->Start(250)) {
            return;
        }
    }

    // TODO error
}

void Engine::stop() {
    reader->Stop();

    process->send("QUIT");
    const auto result {process->join()};

    if (!result) {
        // TODO error
    } else {
        if (*result != 0) {
            // TODO error
        }
    }
}
