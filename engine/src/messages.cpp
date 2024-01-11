#include "messages.hpp"

#include <iostream>

namespace messages {
    void errorcommand(const std::optional<std::string>& message) {
        if (message) {
            std::cout << "ERRORCOMMAND " << *message << '\n';
        } else {
            std::cout << "ERRORCOMMAND\n";
        }
    }
}
