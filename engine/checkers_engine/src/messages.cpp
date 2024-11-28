#include "messages.hpp"

#include <iostream>
#include <sstream>
#include <mutex>
#include <cassert>

namespace messages {
    static std::mutex g_mutex;

    static std::string move_to_string(const game::Move& move) {
        std::ostringstream stream;

        switch (move.type) {
            case game::MoveType::Normal:
                stream << static_cast<int>(game::_0_31_to_1_32(move.normal.source_index))
                    << 'x' << static_cast<int>(game::_0_31_to_1_32(move.normal.destination_index));

                break;
            case game::MoveType::Capture:
                stream << static_cast<int>(game::_0_31_to_1_32(move.capture.source_index));

                for (unsigned char i {0}; i < move.capture.destination_indices_size; i++) {
                    stream << 'x' << static_cast<int>(game::_0_31_to_1_32(move.capture.destination_indices[i]));
                }

                break;
        }

        return stream.str();
    }

    /*
        Using std::endl is crucial.

        Because multiple threads may print messages and because these are split between multiple << calls,
        every message function needs to be protected by a mutex.
    */

    void ready() {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "READY" << std::endl;
    }

    void bestmove(const std::optional<game::Move>& move) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "BESTMOVE ";

        if (move) {
            std::cout << move_to_string(*move);
        } else {
            std::cout << "none";
        }

        std::cout << std::endl;
    }

    void parameters(const std::unordered_map<std::string, parameters::Parameter>& parameters) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "PARAMETERS";

        for (const auto& [name, _] : parameters) {
            std::cout << ' ' << name;
        }

        std::cout << std::endl;
    }

    void parameter(const std::string& name, const parameters::Parameter& value) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "PARAMETER " << name;

        switch (value.index()) {  // Too bad that I can't just use typeid
            case 0:
                std::cout << " int " << std::get<0>(value);
                break;
            case 1:
                std::cout << " float " << std::get<1>(value);
                break;
            case 2:
                std::cout << " bool " << std::get<2>(value);
                break;
            case 3:
                std::cout << " string " << std::get<3>(value);
                break;
            default:
                assert(false);
                break;
        }

        std::cout << std::endl;
    }

    void info(unsigned int nodes, unsigned int transpositions, unsigned int depth, evaluation::Eval eval, double time) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "INFO ";
        std::cout << "nodes " << nodes << ' ';
        std::cout << "transpositions " << transpositions << ' ';
        std::cout << "depth " << depth << ' ';
        std::cout << "eval " << eval << ' ';
        std::cout << "time " << time;

        std::cout << std::endl;
    }

    void name() {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "NAME " << "checkers|3.0" << std::endl;
    }
}
