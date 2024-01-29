#include "messages.hpp"

#include <iostream>
#include <string>
#include <sstream>

namespace messages {
    static std::string move_to_string(const game::Move& move) {
        std::ostringstream stream;

        switch (move.type) {
            case game::MoveType::Normal:
                stream << move.normal.source_index + 1 << 'x' << move.normal.destination_index;

                break;
            case game::MoveType::Capture:
                stream << move.capture.source_index + 1;

                for (unsigned char i {0u}; i < move.capture.destination_indices_size; i++) {
                    stream << 'x' << move.capture.destination_indices[i] + 1;
                }

                break;
        }

        return stream.str();
    }

    void warning(const std::optional<std::string>& message) {
        if (message) {
            std::cout << "WARNING " << *message << '\n';
        } else {
            std::cout << "WARNING\n";
        }
    }

    void bestmove(const game::Move& move) {  // TODO does it need synchronization?
        std::cout << "BESTMOVE " << move_to_string(move) << '\n';
    }
}
