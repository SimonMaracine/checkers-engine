#include "messages.hpp"

#include <iostream>

namespace messages {
    void warning(const std::optional<std::string>& message) {
        if (message) {
            std::cout << "WARNING " << *message << '\n';
        } else {
            std::cout << "WARNING\n";
        }
    }
}
