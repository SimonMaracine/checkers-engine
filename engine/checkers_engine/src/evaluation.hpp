#pragma once

#include "search_node.hpp"
#include "parameters.hpp"

namespace evaluation {
    using Eval = int;

    Eval static_evaluation(const search::SearchNode& node, const parameters::SearchParameters& parameters);
}
