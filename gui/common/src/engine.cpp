#include "common/engine.hpp"

#include <cassert>

// FIXME these functions throw and the exceptions must be caught and wait_for() must be called

namespace engine {
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
        } catch (subprocess::Subprocess::Error) {
            throw;
        }

        if (!process.write_to("INIT\n")) {
            throw Error();
        }

        for (unsigned int i {0u}; i < 5u; i++) {
            if (reader.Start(250)) {
                started = true;
                return;
            }
        }

        throw Error();
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
            throw Error();
        }
    }

    void Engine::move(const std::string& move_string) {
        if (!started) {
            return;
        }

        if (!process.write_to("MOVE " + move_string + '\n')) {
            throw Error();
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
            throw Error();
        }
    }

    void Engine::stop() {
        if (!started) {
            return;
        }

        if (!process.write_to("STOP\n")) {
            throw Error();
        }
    }

    void Engine::getparameters() {
        if (!started) {
            return;
        }

        if (!process.write_to("GETPARAMETERS\n")) {
            throw Error();
        }
    }

    void Engine::getparameter(const std::string& name) {
        if (!started) {
            return;
        }

        if (!process.write_to("GETPARAMETER " + name + '\n')) {
            throw Error();
        }
    }

    void Engine::setparameter(const std::string& name, const std::string& value) {
        if (!started) {
            return;
        }

        if (!process.write_to("SETPARAMETER " + name + ' ' + value + '\n')) {
            throw Error();
        }
    }

    void Engine::quit() {
        if (!started) {
            return;
        }

        reader.Stop();

        if (!process.write_to("QUIT\n")) {
            throw Error();
        }

        if (!process.wait_for()) {
            throw Error();
        }

        started = false;
    }
}
