#pragma once

#include <functional>
#include <string>
#include <optional>

#include "wx.hpp"
#include "subprocess.hpp"

namespace engine {
    using ReadCallback = std::function<void(const std::string&, bool)>;

    class EngineReader : public wxTimer {
    public:
        EngineReader(subprocess::Subprocess& process, const ReadCallback& callback)
            : process(process), callback(callback) {}

        EngineReader(const EngineReader&) = delete;
        EngineReader& operator=(const EngineReader&) = delete;
        EngineReader(EngineReader&&) = delete;
        EngineReader& operator=(EngineReader&&) = delete;

        void Notify() override;
    private:
        subprocess::Subprocess& process;  // Reference is okay
        ReadCallback callback;
    };

    class Engine {
    public:
        using Error = subprocess::Error;

        explicit Engine(const ReadCallback& callback)
            : reader(process, callback) {}

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        bool is_started() const { return started; }
        const std::string& get_name() const { return name; }

        void start_engine(const std::string& file_path, const std::string& name);
        void stop_engine();

        void init();
        void newgame(const std::optional<std::string>& fen_string);
        void move(const std::string& move_string);
        void go(bool dont_play_move);
        void stop();
        void getparameters();
        void getparameter(const std::string& name);
        void setparameter(const std::string& name, const std::string& value);
        void quit();
    private:
        void try_terminate();

        subprocess::Subprocess process;
        EngineReader reader;
        std::string name;
        bool started {false};
    };
}
