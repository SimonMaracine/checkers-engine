#pragma once

#include <functional>
#include <string>
#include <memory>

#include <wx/timer.h>

#include "subprocess.hpp"

namespace engine {
    using ReadCallback = std::function<void(const std::string&)>;

    class EngineReader : public wxTimer {
    public:
        EngineReader()
            : wxTimer() {}
        EngineReader(const subprocess::Subprocess* process, const ReadCallback& callback)
            : wxTimer(), process(process), callback(callback) {}

        void Notify() override;
    private:
        const subprocess::Subprocess* process {nullptr};
        ReadCallback callback;
    };

    class Engine {
    public:
        void start(const std::string& file_path, const ReadCallback& callback);
        void stop();
        void newgame();
        void go();
        void move(const std::string& move_string);
    private:
        subprocess::Subprocess process;
        std::unique_ptr<EngineReader> reader;
    };
}
