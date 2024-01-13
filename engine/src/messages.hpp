#pragma once

#include <string>
#include <optional>

namespace messages {
    void warning(const std::optional<std::string>& message = std::nullopt);
}
