#pragma once

#include "search_node.hpp"

namespace evaluation {
    using Eval = int;

    Eval static_evaluation(const search::SearchNode& node);
}
