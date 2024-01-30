#include "search.hpp"

#include <limits>

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
        if (player == game::Player::Black) {
            game::Eval min_evaluation {std::numeric_limits<game::Eval>::max()};

        } else {
            game::Eval max_evaluation {std::numeric_limits<game::Eval>::min()};

        }
    }
}
