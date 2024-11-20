#pragma once

#include <variant>
#include <string>
#include <unordered_map>

namespace parameters {
    using Parameter = std::variant<int, float, bool, std::string>;
    using Parameters = std::unordered_map<std::string, Parameter>;

    struct SearchParameters {
        int piece {};
    };
}
