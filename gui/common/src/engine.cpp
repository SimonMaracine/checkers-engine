#include "common/engine.hpp"

#include <cassert>

namespace engine {
    void EngineReader::Notify() {
        assert(process != nullptr);
        assert(callback);

        std::string message;

        if (!process->read_from(message)) {
            return;
        }

        callback(message);
    }

    void Engine::start(const std::string& file_path, const ReadCallback& callback) {
        try {
            process = std::make_unique<subprocess::Subprocess>(file_path);
        } catch (int) {
            // TODO error
            throw;
        }

        if (!process->write_to("INIT\n")) {
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

        if (!process->write_to("QUIT\n")) {
            // TODO error
        }

        if (!process->wait_for()) {
            // TODO error
        }
    }

    void Engine::go() {
        if (!process->write_to("GO\n")) {
            // TODO error
        }
    }

    void Engine::move() {

    }
}
