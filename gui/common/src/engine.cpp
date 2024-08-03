#include "common/engine.hpp"

#include <cassert>

namespace engine {
    void EngineReader::Notify() {
        assert(callback);

        bool error {false};
        std::optional<std::string> message;

        try {
            message = process.read();
        } catch (const subprocess::Error& e) {
            process.terminate();
            Stop();
            error = true;
            message = std::make_optional(e.what());
        }

        if (error) {
            callback(*message, true);
            return;
        }

        if (message) {
            callback(*message, false);
        }
    }

    void Engine::init(const std::string& file_path) {
        if (started) {
            return;
        }

        process = subprocess::Subprocess(file_path);

        try {
            process.write("INIT\n");
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error init: ") + e.what());
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

        try {
            process.write(message);
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error newgame: ") + e.what());
        }
    }

    void Engine::move(const std::string& move_string) {
        if (!started) {
            return;
        }

        try {
            process.write("MOVE " + move_string + '\n');
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error move: ") + e.what());
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

        try {
            process.write(message);
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error go: ") + e.what());
        }
    }

    void Engine::stop() {
        if (!started) {
            return;
        }

        try {
            process.write("STOP\n");
        } catch (const subprocess::Error& e) {
            process.terminate();
            throw subprocess::Error(std::string("Error stop: ") + e.what());
        }
    }

    void Engine::getparameters() {
        if (!started) {
            return;
        }

        try {
            process.write("GETPARAMETERS\n");
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error getparameters: ") + e.what());
        }
    }

    void Engine::getparameter(const std::string& name) {
        if (!started) {
            return;
        }

        try {
            process.write("GETPARAMETER " + name + '\n');
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error getparameter: ") + e.what());
        }
    }

    void Engine::setparameter(const std::string& name, const std::string& value) {
        if (!started) {
            return;
        }

        try {
            process.write("SETPARAMETER " + name + ' ' + value + '\n');
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error setparameter: ") + e.what());
        }
    }

    void Engine::quit() {
        if (!started) {
            return;
        }

        reader.Stop();

        try {
            process.write("QUIT\n");
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error quit: ") + e.what());
        }

        try {
            process.wait();
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error waiting for subprocess: ") + e.what());
        }

        started = false;
    }

    void Engine::try_terminate() {
        try {
            process.terminate();
        } catch (const subprocess::Error&) {}

        reader.Stop();
        started = false;
    }
}
