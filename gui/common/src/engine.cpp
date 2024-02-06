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
                return;
            }
        }

        throw ERR;
    }

    void Engine::stop() {
        reader.Stop();

        if (!process.write_to("QUIT\n")) {
            throw ERR;
        }

        if (!process.wait_for()) {
            throw ERR;
        }
    }

    void Engine::newgame() {
        if (!process.write_to("NEWGAME\n")) {
            throw ERR;
        }
    }

    void Engine::go() {
        if (!process.write_to("GO\n")) {
            throw ERR;
        }
    }

    void Engine::move(const std::string& move_string) {
        const std::string message {"MOVE " + move_string + '\n'};

        if (!process.write_to(message)) {
            throw ERR;
        }
    }
}
