#include "common/engine.hpp"

#include <cassert>

namespace engine {
    static constexpr int ERR {0};

    void EngineReader::Notify() {
        assert(callback);

        std::string message;

        if (!process.read_from(message)) {
            return;
        }

        callback(message);
    }

    void Engine::start(const std::string& file_path) {
        if (started) {
            return;
        }

        try {
            process = subprocess::Subprocess(file_path);
        } catch (int) {
            throw;
        }

        if (!process.write_to("INIT\n")) {
            throw ERR;
        }

        for (unsigned int i {0u}; i < 5u; i++) {
            if (reader.Start(250)) {
                started = true;
                return;
            }
        }

        throw ERR;
    }

    void Engine::stop() {
        if (!started) {
            return;
        }

        reader.Stop();

        if (!process.write_to("QUIT\n")) {
            throw ERR;
        }

        if (!process.wait_for()) {
            throw ERR;
        }

        started = false;
    }

    void Engine::newgame() {
        if (!started) {
            return;
        }

        if (!process.write_to("NEWGAME\n")) {
            throw ERR;
        }
    }

    void Engine::go() {
        if (!started) {
            return;
        }

        if (!process.write_to("GO\n")) {
            throw ERR;
        }
    }

    void Engine::move(const std::string& move_string) {
        if (!started) {
            return;
        }

        const std::string message {"MOVE " + move_string + '\n'};

        if (!process.write_to(message)) {
            throw ERR;
        }
    }
}
