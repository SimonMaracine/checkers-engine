#pragma once

#include <functional>
#include <string>
#include <optional>

#include <wx/timer.h>

#include "subprocess.hpp"

namespace engine {
    using ReadCallback = std::function<void(const std::string&)>;

    class EngineReader : public wxTimer {
    public:
        EngineReader(const subprocess::Subprocess& process, const ReadCallback& callback)
            : wxTimer(), process(process), callback(callback) {}

        EngineReader(const EngineReader&) = delete;
        EngineReader& operator=(const EngineReader&) = delete;
        EngineReader(EngineReader&&) = delete;
        EngineReader& operator=(EngineReader&&) = delete;

        void Notify() override;
    private:
        const subprocess::Subprocess& process;  // A reference is okay
        ReadCallback callback;
    };

    class Engine {
    public:
        using Error = subprocess::Subprocess::Error;

        Engine(const ReadCallback& callback)
            : reader(process, callback) {}

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        void init(const std::string& file_path);
        void newgame(const std::optional<std::string>& fen_string);
        void move(const std::string& move_string);
        void go(bool dont_play_move);
        void stop();
        void getparameters();
        void getparameter(const std::string& name);
        void setparameter(const std::string& name, const std::string& value);
        void quit();
    private:
        subprocess::Subprocess process;
        EngineReader reader;
        bool started {false};
    };
}
