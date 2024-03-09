#include "messages.hpp"

#include <iostream>
#include <sstream>
#include <cassert>
#include <mutex>

namespace messages {
    static std::mutex g_mutex;

    static std::string move_to_string(const game::Move& move) {
        std::ostringstream stream;

        switch (move.type) {
            case game::MoveType::Normal:
                stream << static_cast<int>(game::to_1_32(move.normal.source_index))
                    << 'x' << static_cast<int>(game::to_1_32(move.normal.destination_index));

                break;
            case game::MoveType::Capture:
                stream << static_cast<int>(game::to_1_32(move.capture.source_index));

                for (unsigned char i {0u}; i < move.capture.destination_indices_size; i++) {
                    stream << 'x' << static_cast<int>(game::to_1_32(move.capture.destination_indices[i]));
                }

                break;
        }

        return stream.str();
    }

    /*
        Using std::endl is crucial

        Because multiple threads may print messages and because these are split between multiple << calls,
        every message function needs to be protected by a mutex
    */

    void bestmove(const game::Move& move) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "BESTMOVE " << move_to_string(move) << std::endl;
    }

    void parameters(const std::unordered_map<std::string, engine::Param>& parameters) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "PARAMETERS";

        for (const auto& [name, value] : parameters) {
            std::cout << ' ' << name << ' ';

            switch (value.index()) {
                case 0u:
                    std::cout << "int";  // Too bad that I can't just use typeid
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        std::cout << std::endl;
    }

    void parameter(const std::string& name, const engine::Param& value) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "PARAMETER " << name << ' ';

        switch (value.index()) {
            case 0u:
                std::cout << std::get<0u>(value);
                break;
            default:
                assert(false);
                break;
        }

        std::cout << std::endl;
    }
}
