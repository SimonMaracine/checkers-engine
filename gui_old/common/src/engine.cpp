#include "common/engine.hpp"

#include <cassert>

namespace engine {
    void EngineReader::Notify() {
        assert(m_callback);

        bool error {false};
        std::optional<std::string> message;

        try {
            message = m_process.read();
        } catch (const subprocess::Error& e) {
            m_process.terminate();
            Stop();
            error = true;
            message = std::make_optional(e.what());
        }

        if (error) {
            m_callback(*message, true);
            return;
        }

        if (message) {
            m_callback(*message, false);
        }
    }

    void Engine::start_engine(const std::string& file_path, const std::string& name) {
        if (m_started) {
            return;
        }

        m_name = name;

        m_process = subprocess::Subprocess(file_path);

        for (unsigned int i {0}; i < 5; i++) {
            if (m_reader.Start(100)) {
                m_started = true;
                return;
            }
        }

        throw Error("Could not start wxTimer");
    }

    void Engine::stop_engine() {
        if (!m_started) {
            return;
        }

        m_reader.Stop();

        try {
            m_process.wait();
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error waiting for subprocess: ") + e.what());
        }

        m_started = false;
    }

    void Engine::init() {
        if (!m_started) {
            return;
        }

        try {
            m_process.write("INIT\n");
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error init: ") + e.what());
        }
    }

    void Engine::newgame(const std::optional<std::string>& fen_string) {
        if (!m_started) {
            return;
        }

        std::string message;

        if (fen_string) {
            message = "NEWGAME " + *fen_string + '\n';
        } else {
            message = "NEWGAME\n";
        }

        try {
            m_process.write(message);
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error newgame: ") + e.what());
        }
    }

    void Engine::move(const std::string& move_string) {
        if (!m_started) {
            return;
        }

        try {
            m_process.write("MOVE " + move_string + '\n');
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error move: ") + e.what());
        }
    }

    void Engine::go(bool dont_play_move) {
        if (!m_started) {
            return;
        }

        std::string message;

        if (dont_play_move) {
            message = "GO dontplaymove\n";
        } else {
            message = "GO\n";
        }

        try {
            m_process.write(message);
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error go: ") + e.what());
        }
    }

    void Engine::stop() {
        if (!m_started) {
            return;
        }

        try {
            m_process.write("STOP\n");
        } catch (const subprocess::Error& e) {
            m_process.terminate();
            throw subprocess::Error(std::string("Error stop: ") + e.what());
        }
    }

    void Engine::getparameters() {
        if (!m_started) {
            return;
        }

        try {
            m_process.write("GETPARAMETERS\n");
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error getparameters: ") + e.what());
        }
    }

    void Engine::getparameter(const std::string& name) {
        if (!m_started) {
            return;
        }

        try {
            m_process.write("GETPARAMETER " + name + '\n');
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error getparameter: ") + e.what());
        }
    }

    void Engine::setparameter(const std::string& name, const std::string& value) {
        if (!m_started) {
            return;
        }

        try {
            m_process.write("SETPARAMETER " + name + ' ' + value + '\n');
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error setparameter: ") + e.what());
        }
    }

    void Engine::quit() {
        if (!m_started) {
            return;
        }

        try {
            m_process.write("QUIT\n");
        } catch (const subprocess::Error& e) {
            try_terminate();
            throw subprocess::Error(std::string("Error quit: ") + e.what());
        }
    }

    void Engine::try_terminate() {
        try {
            m_process.terminate();
        } catch (const subprocess::Error&) {}

        m_reader.Stop();
        m_started = false;
    }
}
