#pragma once

#include <string>
#include <optional>

namespace messages {
    void errorcommand(const std::optional<std::string>& message = std::nullopt);
}
