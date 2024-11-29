#include "moves.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>
#include <array>
#include <cassert>

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

    enum Diagonal {
        Short = 1,
        Long = 2
    };

    struct JumpCtx {
        game::Board board {};  // Use a copy of the board
        game::Idx source_index {};
        std::vector<game::Idx> destination_indices;
    };

    static game::Idx offset(game::Idx square_index, Direction direction, Diagonal diagonal) {
        int result_index {square_index};

        const bool even_row {(square_index / 4) % 2 == 0};

        switch (direction) {  // TODO opt.
            case Direction::NorthEast:
                result_index -= even_row ? 3 : 4;

                if (diagonal == Diagonal::Long) {
                    result_index -= even_row ? 4 : 3;
                }

                break;
            case Direction::NorthWest:
                result_index -= even_row ? 4 : 5;

                if (diagonal == Diagonal::Long) {
                    result_index -= even_row ? 5 : 4;
                }

                break;
            case Direction::SouthEast:
                result_index += even_row ? 5 : 4;

                if (diagonal == Diagonal::Long) {
                    result_index += even_row ? 4 : 5;
                }

                break;
            case Direction::SouthWest:
                result_index += even_row ? 4 : 3;

                if (diagonal == Diagonal::Long) {
                    result_index += even_row ? 3 : 4;
                }

                break;
        }

        // Check edge cases (literally)
        if (std::abs(square_index / 4 - result_index / 4) != static_cast<int>(diagonal)) {
            return game::NULL_INDEX;
        }

        // Check out of bounds
        if (result_index < 0 || result_index > 31) {
            return game::NULL_INDEX;
        }

        return static_cast<game::Idx>(result_index);
    }

    static std::array<Direction, 4> get_directions(game::Player player, bool king, std::size_t& size) {
        if (king) {
            size = 4;

            return {
                Direction::NorthEast,
                Direction::NorthWest,
                Direction::SouthEast,
                Direction::SouthWest
            };
        } else {
            size = 2;

            switch (player) {
                case game::Player::Black:
                    return {
                        Direction::SouthEast,
                        Direction::SouthWest
                    };
                case game::Player::White:
                    return {
                        Direction::NorthEast,
                        Direction::NorthWest
                    };
            }
        }

        assert(false);
        return {};
    }

    static bool check_piece_jumps(
        std::vector<game::Move>& moves,
        game::Idx square_index,
        game::Player player,
        bool king,
        JumpCtx& ctx
    ) {
        std::size_t size {};
        const auto directions {get_directions(player, king, size)};

        // We want an enemy piece
        const unsigned char piece_mask {static_cast<unsigned char>(game::opponent(player))};

        bool sequence_jumps_ended {true};

        for (auto iter {std::begin(directions)}; iter != std::next(std::begin(directions), size); iter++) {
            const game::Idx enemy_index {offset(square_index, *iter, Short)};
            const game::Idx target_index {offset(square_index, *iter, Long)};

            if (enemy_index == game::NULL_INDEX || target_index == game::NULL_INDEX) {
                continue;
            }

            const bool is_enemy_piece {static_cast<bool>(static_cast<unsigned char>(ctx.board[enemy_index]) & piece_mask)};

            if (!is_enemy_piece || ctx.board[target_index] != game::Square::None) {
                continue;
            }

            sequence_jumps_ended = false;

            ctx.destination_indices.push_back(target_index);

            // Remove the piece to avoid illegal jumps
            const auto removed_enemy_piece {std::exchange(ctx.board[enemy_index], game::Square::None)};

            // Jump this piece to avoid other illegal jumps
            std::swap(ctx.board[square_index], ctx.board[target_index]);

            if (check_piece_jumps(moves, target_index, player, king, ctx)) {
                // This means that it reached the end of a sequence of jumps; the piece can't jump anymore

                game::Move move;
                move.type = game::MoveType::Capture;
                move.capture.source_index = ctx.source_index;
                move.capture.destination_indices_size = static_cast<unsigned char>(ctx.destination_indices.size());

                for (std::size_t i {0}; i < ctx.destination_indices.size(); i++) {
                    move.capture.destination_indices[i] = ctx.destination_indices[i];
                }

                moves.push_back(move);
            }

            // Restore jumped piece
            std::swap(ctx.board[square_index], ctx.board[target_index]);

            // Restore removed piece
            ctx.board[enemy_index] = removed_enemy_piece;

            ctx.destination_indices.pop_back();
        }

        return sequence_jumps_ended;
    }

    static void generate_piece_capture_moves(
        const game::Board& board,
        game::Player player,
        game::Idx square_index,
        bool king,
        std::vector<game::Move>& moves
    ) {
        JumpCtx ctx;
        ctx.board = board;
        ctx.source_index = square_index;

        check_piece_jumps(moves, square_index, player, king, ctx);
    }

    static void generate_piece_moves(
        const game::Board& board,
        game::Player player,
        game::Idx square_index,
        bool king,
        std::vector<game::Move>& moves
    ) {
        std::size_t size {};
        const auto directions {get_directions(player, king, size)};

        // Check the squares above or below in diagonal
        for (auto iter {std::begin(directions)}; iter != std::next(std::begin(directions), size); iter++) {
            const game::Idx target_index {offset(square_index, *iter, Short)};

            if (target_index == game::NULL_INDEX) {
                continue;
            }

            if (board[target_index] != game::Square::None) {
                continue;
            }

            game::Move move;
            move.type = game::MoveType::Normal;
            move.normal.source_index = square_index;
            move.normal.destination_index = target_index;

            moves.push_back(move);
        }
    }

    std::vector<game::Move> generate_moves(const game::Board& board, game::Player player) {
        std::vector<game::Move> moves;

        for (game::Idx i {0}; i < 32; i++) {
            if (game::is_piece(board[i], player)) {
                generate_piece_capture_moves(board, player, i, game::is_king_piece(board[i]), moves);
            }
        }

        // If there are possible captures, force the player to play these moves
        if (!moves.empty()) {
            return moves;
        }

        for (game::Idx i {0}; i < 32; i++) {
            if (game::is_piece(board[i], player)) {
                generate_piece_moves(board, player, i, game::is_king_piece(board[i]), moves);
            }
        }

        return moves;
    }
}
