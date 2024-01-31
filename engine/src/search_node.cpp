#include "search_node.hpp"

namespace search {
    void fill_node(SearchNode& current, const SearchNode& previous) {
        current.board = previous.board;
        current.plies = previous.plies;
        current.plies_without_advancement = previous.plies_without_advancement;

        current.previous = &previous;
    }
}
