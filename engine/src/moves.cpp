#include "moves.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <utility>

namespace moves {
    static game::Idx get_jumped_piece_index(game::Idx index1, game::Idx index2) {
        // This works with indices in the range [1, 32]

        const int sum {index1 + index2};

        assert(sum % 2 == 1);

        if (((game::to_0_31(index1)) / 4) % 2 == 0) {
            return static_cast<game::Idx>((sum + 1) / 2);
        } else {
            return static_cast<game::Idx>((sum - 1) / 2);
        }
    }

    static void remove_jumped_pieces(game::Board& board, const game::Move& move) {
        assert(move.type == game::MoveType::Capture);

        assert(board[move.capture.destination_indices[0u]] == game::Square::None);

        const auto index {get_jumped_piece_index(
            game::to_1_32(move.capture.source_index),
            game::to_1_32(move.capture.destination_indices[0u])
        )};

        assert(board[game::to_0_31(index)] != game::Square::None);

        board[game::to_0_31(index)] = game::Square::None;

        for (unsigned char i {0u}; i < move.capture.destination_indices_size - 1u; i++) {
            assert(board[move.capture.destination_indices[i + 1u]] == game::Square::None);

            const auto index {get_jumped_piece_index(
                game::to_1_32(move.capture.destination_indices[i]),
                game::to_1_32(move.capture.destination_indices[i + 1u])
            )};

            assert(board[game::to_0_31(index)] != game::Square::None);

            board[game::to_0_31(index)] = game::Square::None;
        }
    }

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
        game::Idx result_index {square_index};

        const bool even_row {(square_index / 4) % 2 == 0};

        switch (direction) {  // TODO opt.
            case Direction::NorthEast:
                result_index -= even_row ? 3 : 4;  // TODO fix warnings

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

        return result_index;
    }

    static bool check_piece_jumps(std::vector<game::Move>& moves, game::Idx square_index, game::Player player, bool king, JumpCtx& ctx) {
        Direction directions[4u] {};
        std::size_t index {0u};

        if (king) {
            directions[index++] = Direction::NorthEast;
            directions[index++] = Direction::NorthWest;
            directions[index++] = Direction::SouthEast;
            directions[index++] = Direction::SouthWest;
        } else {
            switch (player) {
                case game::Player::Black:
                    directions[index++] = Direction::NorthEast;
                    directions[index++] = Direction::NorthWest;
                    break;
                case game::Player::White:
                    directions[index++] = Direction::SouthEast;
                    directions[index++] = Direction::SouthWest;
                    break;
            }
        }

        // We want an enemy piece
        const unsigned char piece_mask {static_cast<unsigned char>(game::opponent(player))};

        bool sequence_jumps_ended {true};

        for (auto iter {std::begin(directions)}; iter != std::next(std::begin(directions), index); iter++) {
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

                for (std::size_t i {0u}; i < ctx.destination_indices.size(); i++) {
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

    static void generate_piece_capture_moves(const game::Board& board, game::Player player, game::Idx square_index, bool king, std::vector<game::Move>& moves) {
        JumpCtx ctx;
        ctx.board = board;
        ctx.source_index = square_index;

        check_piece_jumps(moves, square_index, player, king, ctx);
    }

    static void generate_piece_moves(const game::Board& board, game::Player player, game::Idx square_index, bool king, std::vector<game::Move>& moves) {
        Direction directions[4u] {};
        std::size_t index {0u};

        if (king) {
            directions[index++] = Direction::NorthEast;
            directions[index++] = Direction::NorthWest;
            directions[index++] = Direction::SouthEast;
            directions[index++] = Direction::SouthWest;
        } else {
            switch (player) {
                case game::Player::Black:
                    directions[index++] = Direction::NorthEast;
                    directions[index++] = Direction::NorthWest;
                    break;
                case game::Player::White:
                    directions[index++] = Direction::SouthEast;
                    directions[index++] = Direction::SouthWest;
                    break;
            }
        }

        // Check the squares above or below in diagonal
        for (auto iter {std::begin(directions)}; iter != std::next(std::begin(directions), index); iter++) {
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

    static void check_piece_crowning(game::Board& board, game::Idx square_index) {
        const int row {square_index / 4};

        if (game::is_black_piece(board[square_index])) {
            if (row == 0) {
                board[square_index] = game::Square::BlackKing;
            }
        } else {  // Assume it's white
            if (row == 7) {
                board[square_index] = game::Square::WhiteKing;
            }
        }
    }

    void play_move(game::Position& position, const game::Move& move) {
        auto& board {position.position.board};

        switch (move.type) {
            case game::MoveType::Normal:
                assert(board[move.normal.destination_index] == game::Square::None);  // FIXME this failed in endgame when I switched from computer to human

                std::swap(board[move.normal.source_index], board[move.normal.destination_index]);

                check_piece_crowning(board, move.normal.destination_index);

                break;
            case game::MoveType::Capture:
                assert(board[move.capture.destination_indices[move.capture.destination_indices_size - 1u]] == game::Square::None);

                remove_jumped_pieces(board, move);
                std::swap(
                    board[move.capture.source_index],
                    board[move.capture.destination_indices[move.capture.destination_indices_size - 1u]]
                );

                check_piece_crowning(board, move.capture.destination_indices[move.capture.destination_indices_size - 1u]);

                break;
        }

        position.position.player = game::opponent(position.position.player);
        position.plies++;
    }

    void play_move(search::SearchNode& node, const game::Move& move) {
        switch (move.type) {
            case game::MoveType::Normal:
                assert(node.board[move.normal.destination_index] == game::Square::None);

                std::swap(node.board[move.normal.source_index], node.board[move.normal.destination_index]);

                check_piece_crowning(node.board, move.normal.destination_index);

                break;
            case game::MoveType::Capture:
                assert(node.board[move.capture.destination_indices[move.capture.destination_indices_size - 1u]] == game::Square::None);

                remove_jumped_pieces(node.board, move);
                std::swap(
                    node.board[move.capture.source_index],
                    node.board[move.capture.destination_indices[move.capture.destination_indices_size - 1u]]
                );

                check_piece_crowning(node.board, move.capture.destination_indices[move.capture.destination_indices_size - 1u]);

                break;
        }

        node.plies++;
    }

    std::vector<game::Move> generate_moves(const game::Board& board, game::Player player) {
        std::vector<game::Move> moves;

        for (game::Idx i {0}; i < 32; i++) {
            const bool king {static_cast<bool>(static_cast<unsigned char>(board[i]) & (1u << 2))};
            const bool piece {static_cast<bool>(static_cast<unsigned char>(board[i]) & static_cast<unsigned char>(player))};

            if (piece) {
                generate_piece_capture_moves(board, player, i, king, moves);
            }
        }

        // If there are possible captures, force the player to play these moves
        if (!moves.empty()) {
            return moves;
        }

        for (game::Idx i {0}; i < 32; i++) {
            const bool king {static_cast<bool>(static_cast<unsigned char>(board[i]) & (1u << 2))};
            const bool piece {static_cast<bool>(static_cast<unsigned char>(board[i]) & static_cast<unsigned char>(player))};

            if (piece) {
                generate_piece_moves(board, player, i, king, moves);
            }
        }

        return moves;
    }
}
