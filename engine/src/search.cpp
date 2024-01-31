#include "search.hpp"

#include <limits>

#include "moves.hpp"

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

            const auto moves {moves::generate_moves(ctx.board, player)};

            for (const game::Move& move : moves) {
                moves::play_move(ctx.board, player, ctx.plies, ctx.plies_without_advancement, move);

            }

        } else {
            game::Eval max_evaluation {std::numeric_limits<game::Eval>::min()};

        }
    }
}
