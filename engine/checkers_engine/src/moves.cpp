#include "moves.hpp"

#include <algorithm>
#include <utility>
#include <type_traits>
#include <cassert>

#include "utils.hpp"

/*
    capture loop W:B1,2,3,26,18,17,25:WK30,28
*/

namespace moves {
    enum class Direction {
        NorthEast,
        NorthWest,
        SouthEast,
        SouthWest
    };

    enum Diagonal : int {
        Short = 1,
        Long = 2
    };

    struct JumpCtx {
        game::Board board {};  // Use a copy of the board
        int source_index {};
        array::Array<int, 9> destination_indices;
    };

    template<Direction Dir, Diagonal Diag>
    static int offset(int square_index) noexcept {
        int result_index {square_index};

        const bool even_row {(square_index / 4) % 2 == 0};

        switch (Dir) {
            case Direction::NorthEast:
                result_index -= even_row ? 3 : 4;

                if (Diag == Diagonal::Long) {
                    result_index -= even_row ? 4 : 3;
                }

                break;
            case Direction::NorthWest:
                result_index -= even_row ? 4 : 5;

                if (Diag == Diagonal::Long) {
                    result_index -= even_row ? 5 : 4;
                }

                break;
            case Direction::SouthEast:
                result_index += even_row ? 5 : 4;

                if (Diag == Diagonal::Long) {
                    result_index += even_row ? 4 : 5;
                }

                break;
            case Direction::SouthWest:
                result_index += even_row ? 4 : 3;

                if (Diag == Diagonal::Long) {
                    result_index += even_row ? 3 : 4;
                }

                break;
        }

        // Check edge cases (literally)
        if (std::abs(square_index / 4 - result_index / 4) != Diag) {
            return game::NULL_INDEX;
        }

        // Check out of bounds
        if (result_index < 0 || result_index > 31) {
            return game::NULL_INDEX;
        }

        return result_index;
    }

    static bool check_piece_jumps(JumpCtx& ctx, int square_index, game::Player player, bool king, Moves& moves) noexcept;

    template<Direction Dir, bool King>
    static void check_square_capture_move(JumpCtx& ctx, int square_index, game::Player player, std::underlying_type_t<game::Square> piece_mask, bool& sequence_jumps_ended, Moves& moves) noexcept {
        const int enemy_index {offset<Dir, Short>(square_index)};
        const int target_index {offset<Dir, Long>(square_index)};

        if (enemy_index == game::NULL_INDEX || target_index == game::NULL_INDEX) {
            return;
        }

        const bool is_enemy_piece {static_cast<bool>(static_cast<std::underlying_type_t<game::Square>>(ctx.board[enemy_index]) & piece_mask)};

        if (!is_enemy_piece || ctx.board[target_index] != game::Square::None) {
            return;
        }

        sequence_jumps_ended = false;

        ctx.destination_indices.push_back(target_index);

        // Remove the piece to avoid illegal jumps
        const auto removed_enemy_piece {std::exchange(ctx.board[enemy_index], game::Square::None)};

        // Jump this piece to avoid other illegal jumps
        std::swap(ctx.board[square_index], ctx.board[target_index]);

        if (check_piece_jumps(ctx, target_index, player, King, moves)) {
            // This means that it reached the end of a sequence of jumps; the piece can't jump anymore

            game::Move::DestinationIndices destination_indices {};

            for (int i {0}; i < ctx.destination_indices.size(); i++) {
                destination_indices[i] = ctx.destination_indices[i];
            }

            moves.emplace_back(ctx.source_index, destination_indices, ctx.destination_indices.size());
        }

        // Restore jumped piece
        std::swap(ctx.board[square_index], ctx.board[target_index]);

        // Restore removed piece
        ctx.board[enemy_index] = removed_enemy_piece;

        ctx.destination_indices.pop_back();
    }

    static bool check_piece_jumps(JumpCtx& ctx, int square_index, game::Player player, bool king, Moves& moves) noexcept {
        // We want an enemy piece
        const auto piece_mask {static_cast<std::underlying_type_t<game::Player>>(game::opponent(player))};

        bool sequence_jumps_ended {true};

        // Check the squares above or below in diagonal
        switch (static_cast<std::underlying_type_t<game::Player>>(player) | static_cast<std::underlying_type_t<game::Player>>(king) << 2) {
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::None):
                assert(false);
                utils::unreachable();
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::Black):
                check_square_capture_move<Direction::SouthEast, false>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                check_square_capture_move<Direction::SouthWest, false>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                break;
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::White):
                check_square_capture_move<Direction::NorthEast, false>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                check_square_capture_move<Direction::NorthWest, false>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                break;
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::WhiteKing):
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::BlackKing):
                check_square_capture_move<Direction::NorthEast, true>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                check_square_capture_move<Direction::NorthWest, true>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                check_square_capture_move<Direction::SouthEast, true>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                check_square_capture_move<Direction::SouthWest, true>(ctx, square_index, player, piece_mask, sequence_jumps_ended, moves);
                break;
        }

        return sequence_jumps_ended;
    }

    static void generate_piece_capture_moves(const game::Board& board, game::Player player, int square_index, bool king, Moves& moves) noexcept {
        JumpCtx ctx;
        ctx.board = board;
        ctx.source_index = square_index;

        check_piece_jumps(ctx, square_index, player, king, moves);
    }

    template<Direction Dir>
    static void check_square_normal_move(const game::Board& board, int square_index, Moves& moves) noexcept {
        const int target_index {offset<Dir, Short>(square_index)};

        if (target_index == game::NULL_INDEX) {
            return;
        }

        if (board[target_index] != game::Square::None) {
            return;
        }

        moves.emplace_back(square_index, target_index);
    }

    static void generate_piece_normal_moves(const game::Board& board, game::Player player, int square_index, bool king, Moves& moves) noexcept {
        // Check the squares above or below in diagonal
        switch (static_cast<std::underlying_type_t<game::Player>>(player) | static_cast<std::underlying_type_t<game::Player>>(king) << 2) {
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::None):
                assert(false);
                utils::unreachable();
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::Black):
                check_square_normal_move<Direction::SouthEast>(board, square_index, moves);
                check_square_normal_move<Direction::SouthWest>(board, square_index, moves);
                break;
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::White):
                check_square_normal_move<Direction::NorthEast>(board, square_index, moves);
                check_square_normal_move<Direction::NorthWest>(board, square_index, moves);
                break;
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::WhiteKing):
            case static_cast<std::underlying_type_t<game::Square>>(game::Square::BlackKing):
                check_square_normal_move<Direction::NorthEast>(board, square_index, moves);
                check_square_normal_move<Direction::NorthWest>(board, square_index, moves);
                check_square_normal_move<Direction::SouthEast>(board, square_index, moves);
                check_square_normal_move<Direction::SouthWest>(board, square_index, moves);
                break;
        }
    }

    Moves generate_moves(const game::Board& board, game::Player player) noexcept {
        Moves moves;

        for (int i {0}; i < 32; i++) {
            if (game::is_piece(board[i], player)) {
                generate_piece_capture_moves(board, player, i, game::is_king_piece(board[i]), moves);
            }
        }

        // If there are possible captures, force the player to play these moves
        if (!moves.empty()) {
            return moves;
        }

        for (int i {0}; i < 32; i++) {
            if (game::is_piece(board[i], player)) {
                generate_piece_normal_moves(board, player, i, game::is_king_piece(board[i]), moves);
            }
        }

        return moves;
    }
}
