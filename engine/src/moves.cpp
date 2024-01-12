#include "moves.hpp"

#include <algorithm>
#include <cassert>

namespace moves {
    static game::Idx get_jumped_piece_index(game::Idx index1, game::Idx index2) {
        const auto sum {index1 + index2};

        assert(sum % 2 == 1);

        if (((index1 - 1) / 4) % 2 == 0) {
            return (sum + 1) / 2;
        } else {
            return (sum - 1) / 2;
        }
    }

    static void remove_jumped_pieces(game::Position& position, const game::Move& move) {  // FIXME
        assert(move.type == game::MoveType::Capture);

        const auto index {get_jumped_piece_index(
            move.capture.source_index + 1,
            move.capture.destination_indices[0u] + 1
        )};
        position.board[index - 1] = game::Square::None;

        for (unsigned char i {0u}; i < move.capture.destination_indices_size - 1u; i++) {
            const auto index {get_jumped_piece_index(
                move.capture.destination_indices[i] + 1,
                move.capture.destination_indices[i + 1u] + 1
            )};
            position.board[index - 1] = game::Square::None;
        }
    }

    void play_move(game::Position& position, const game::Move& move) {
        switch (move.type) {
            case game::MoveType::Normal:
                std::swap(position.board[move.normal.source_index], position.board[move.normal.destination_index]);

                break;
            case game::MoveType::Capture:
                std::swap(
                    position.board[move.capture.source_index],
                    position.board[move.capture.destination_indices[move.capture.destination_indices_size - 1u]]
                );
                remove_jumped_pieces(position, move);

                break;
        }
    }
}
