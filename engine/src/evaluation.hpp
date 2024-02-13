#pragma once

#include "search_node.hpp"

namespace evaluation {
    using Eval = int;

    struct Parameters {
        int PIECE {};
        int DEPTH {};
    };

    Eval static_evaluation(const search::SearchNode& node, const evaluation::Parameters& parameters);
}
