#pragma once

#include <limits>

#include "search_node.hpp"
#include "parameters.hpp"

namespace evaluation {
    using Eval = int;

    inline constexpr Eval MAX {1'000'000'000};
    inline constexpr Eval MIN {-MAX};
    inline constexpr Eval INVALID {std::numeric_limits<Eval>::max()};

    Eval static_evaluation(const search::SearchNode& node, const parameters::SearchParameters& parameters);
}
