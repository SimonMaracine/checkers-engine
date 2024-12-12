#pragma once

#include <limits>

#include "game.hpp"
#include "parameters.hpp"

namespace evaluation {
    using Eval = int;

    // These should be used only for the window
    // Normal evaluation values must not reach max or min window values
    inline constexpr Eval WINDOW_MAX {1'000'000'000};
    inline constexpr Eval WINDOW_MIN {-WINDOW_MAX};

    // Max or min evaluation values
    inline constexpr Eval MAX {WINDOW_MAX - 1};
    inline constexpr Eval MIN {WINDOW_MIN + 1};

    // Null evaluation token used by TT
    inline constexpr Eval UNKNOWN {std::numeric_limits<Eval>::max()};

    Eval static_evaluation(const game::SearchNode& node, const parameters::SearchParameters& parameters) noexcept;
}
