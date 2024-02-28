#include "common/engine.hpp"

#include <cassert>

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

        process = subprocess::Subprocess(file_path);

        if (!process.write_to("INIT\n")) {
            throw Error("Could not write all");
        }

        for (unsigned int i {0u}; i < 5u; i++) {
            if (reader.Start(250)) {
                started = true;
                return;
            }
        }

        throw Error("Could not start wxTimer");
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
            throw Error("Could not write all");
        }
    }

    void Engine::move(const std::string& move_string) {
        if (!started) {
            return;
        }

        if (!process.write_to("MOVE " + move_string + '\n')) {
            throw Error("Could not write all");
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
            throw Error("Could not write all");
        }
    }

    void Engine::stop() {
        if (!started) {
            return;
        }

        if (!process.write_to("STOP\n")) {
            throw Error("Could not write all");
        }
    }

    void Engine::getparameters() {
        if (!started) {
            return;
        }

        if (!process.write_to("GETPARAMETERS\n")) {
            throw Error("Could not write all");
        }
    }

    void Engine::getparameter(const std::string& name) {
        if (!started) {
            return;
        }

        if (!process.write_to("GETPARAMETER " + name + '\n')) {
            throw Error("Could not write all");
        }
    }

    void Engine::setparameter(const std::string& name, const std::string& value) {
        if (!started) {
            return;
        }

        if (!process.write_to("SETPARAMETER " + name + ' ' + value + '\n')) {
            throw Error("Could not write all");
        }
    }

    void Engine::quit() {
        if (!started) {
            return;
        }

        reader.Stop();

        try {
            if (!process.write_to("QUIT\n")) {
                throw Error("Could not write all");
            }
        } catch (const subprocess::Subprocess::Error&) {
            process.wait_for();
            throw;
        }

        if (!process.wait_for()) {
            throw Error("Could not wait for subprocess");
        }

        started = false;
    }
}
