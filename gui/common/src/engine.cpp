#include "common/engine.hpp"

#include <cassert>

// FIXME these functions throw and the exceptions must be caught and wait_for() must be called

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

    void Engine::init(const std::string& file_path) {
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

    void Engine::newgame(const std::optional<std::string>& fen_string) {
        if (!started) {
            return;
        }

        std::string message;

        if (fen_string) {
            message = "NEWGAME " + *fen_string + '\n';
        } else {
            message = "NEWGAME\n";
        }

        if (!process.write_to(message)) {
            throw ERR;
        }
    }

    void Engine::move(const std::string& move_string) {
        if (!started) {
            return;
        }

        if (!process.write_to("MOVE " + move_string + '\n')) {
            throw ERR;
        }
    }

    void Engine::go(bool dont_play_move) {
        if (!started) {
            return;
        }

        std::string message;

        if (dont_play_move) {
            message = "GO dontplaymove\n";
        } else {
            message = "GO\n";
        }

        if (!process.write_to(message)) {
            throw ERR;
        }
    }

    void Engine::stop() {
        if (!started) {
            return;
        }

        if (!process.write_to("STOP\n")) {
            throw ERR;
        }
    }

    void Engine::getparameters() {
        if (!started) {
            return;
        }

        if (!process.write_to("GETPARAMETERS\n")) {
            throw ERR;
        }
    }

    void Engine::getparameter(const std::string& name) {
        if (!started) {
            return;
        }

        if (!process.write_to("GETPARAMETER " + name + '\n')) {
            throw ERR;
        }
    }

    void Engine::setparameter(const std::string& name, const std::string& value) {
        if (!started) {
            return;
        }

        if (!process.write_to("SETPARAMETER " + name + ' ' + value + '\n')) {
            throw ERR;
        }
    }

    void Engine::quit() {
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
}
