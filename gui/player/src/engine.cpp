#include "engine.hpp"

#include <cassert>

void EngineReader::Notify() {
    assert(process != nullptr);
    assert(callback);

    const auto message {process->receive()};

    if (!message) {
        return;
    }

    callback(*message);
}

void Engine::start(const std::string& file_path, const ReadCallback& callback) {
    process = Subprocess(file_path);
    process.send("INIT\n");

    reader = std::make_unique<EngineReader>(&process, callback);
    reader->Start();
}

void Engine::stop() {
    reader->Stop();

    process.send("QUIT\n");
    const auto result {process.join()};

    if (!result) {
        // TODO error
    } else {
        if (*result != 0) {
            // TODO error
        }
    }
}
