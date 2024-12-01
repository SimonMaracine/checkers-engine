#pragma once

#include <variant>
#include <string>
#include <unordered_map>

namespace parameters {
    using Int = int;
    using Float = float;
    using Bool = bool;
    using String = std::string;

    using Parameter = std::variant<Int, Float, Bool, String>;
    using Parameters = std::unordered_map<std::string, Parameter>;

    struct SearchParameters {
        Int piece {};
        Int piece_king {};
        Int positioning {};
        Int crowdness {};
    };
}
