#include "search.hpp"

namespace search {
    game::Move Search::search(
        const game::Position& position,
        const std::vector<game::Position>& previous_positions,
        const std::vector<game::Move>& moves_played
    ) {
        return {};
    }

    game::Eval Search::minimax(
        game::Player player,
        unsigned int depth,
        unsigned int plies_from_root
    ) {
        return {};
    }
}
