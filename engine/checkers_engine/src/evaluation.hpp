#pragma once

#include <limits>

#include "parameters.hpp"

namespace search {
    struct SearchNode;
}

namespace evaluation {
    using Eval = int;

    inline constexpr Eval WINDOW_MAX {1'000'000'000};
    inline constexpr Eval WINDOW_MIN {-WINDOW_MAX};
    inline constexpr Eval MAX {WINDOW_MAX - 1};
    inline constexpr Eval MIN {WINDOW_MIN + 1};
    inline constexpr Eval INVALID {std::numeric_limits<Eval>::max()};

    Eval static_evaluation(const search::SearchNode& node, const parameters::SearchParameters& parameters);
}
