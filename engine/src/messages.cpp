#include "messages.hpp"

#include <iostream>
#include <sstream>

namespace messages {
    static std::string move_to_string(const game::Move& move) {
        std::ostringstream stream;

        switch (move.type) {
            case game::MoveType::Normal:
                stream << static_cast<int>(game::to_1_32(move.normal.source_index)) << 'x' << static_cast<int>(game::to_1_32(move.normal.destination_index));

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

    void warning(const std::string& message) {
        if (!message.empty()) {
            std::cout << "WARNING " << message << '\n';
        } else {
            std::cout << "WARNING\n";
        }
    }

    void bestmove(const game::Move& move) {  // TODO does it need synchronization?
        std::cout << "BESTMOVE " << move_to_string(move) << '\n';
    }
}
