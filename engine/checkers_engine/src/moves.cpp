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

    static game::Idx get_jumped_piece_index(game::Idx index1, game::Idx index2) {
        // This works with indices in the range [1, 32]

        const int sum {index1 + index2};

        assert(sum % 2 == 1);

        if (((game::_1_32_to_0_31(index1)) / 4) % 2 == 0) {
            return static_cast<game::Idx>((sum + 1) / 2);
        } else {
            return static_cast<game::Idx>((sum - 1) / 2);
        }
    }

    static void remove_jumped_pieces(game::Board& board, const game::Move& move) {
        assert(move.type == game::MoveType::Capture);

        {
            assert(
                board[move.capture.destination_indices[0]] == game::Square::None ||
                move.capture.source_index == move.capture.destination_indices[0]
            );

            const auto index {get_jumped_piece_index(
                game::_0_31_to_1_32(move.capture.source_index),
                game::_0_31_to_1_32(move.capture.destination_indices[0])
            )};

            assert(board[game::_1_32_to_0_31(index)] != game::Square::None);

            board[game::_1_32_to_0_31(index)] = game::Square::None;
        }

        for (unsigned char i {0}; i < move.capture.destination_indices_size - 1; i++) {
            assert(
                board[move.capture.destination_indices[i + 1]] == game::Square::None ||
                move.capture.source_index == move.capture.destination_indices[i + 1]
            );

            const auto index {get_jumped_piece_index(
                game::_0_31_to_1_32(move.capture.destination_indices[i]),
                game::_0_31_to_1_32(move.capture.destination_indices[i + 1])
            )};

            assert(board[game::_1_32_to_0_31(index)] != game::Square::None);

            board[game::_1_32_to_0_31(index)] = game::Square::None;
        }
    }

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

    static void check_piece_crowning(game::Board& board, game::Idx square_index) {
        const int row {square_index / 4};

        if (game::is_black_piece(board[square_index])) {  // TODO opt.
            if (row == 7) {
                board[square_index] = game::Square::BlackKing;
            }
        } else {  // Assume it's white
            assert(game::is_white_piece(board[square_index]));

            if (row == 0) {
                board[square_index] = game::Square::WhiteKing;
            }
        }
    }

    void play_move(game::Position& position, const game::Move& move) {
        auto& board {position.board};

        switch (move.type) {
            case game::MoveType::Normal:
                assert(board[move.normal.source_index] != game::Square::None);
                assert(board[move.normal.destination_index] == game::Square::None);

                std::swap(board[move.normal.source_index], board[move.normal.destination_index]);

                if (!game::is_king_piece(board[move.normal.destination_index])) {
                    position.plies_without_advancement = 0;
                } else {
                    position.plies_without_advancement++;
                }

                check_piece_crowning(board, move.normal.destination_index);

                break;
            case game::MoveType::Capture:
                assert(board[move.capture.source_index] != game::Square::None);
                assert(
                    board[move.capture.destination_indices[move.capture.destination_indices_size - 1]] == game::Square::None ||
                    move.capture.source_index == move.capture.destination_indices[move.capture.destination_indices_size - 1]
                );

                remove_jumped_pieces(board, move);
                std::swap(
                    board[move.capture.source_index],
                    board[move.capture.destination_indices[move.capture.destination_indices_size - 1]]
                );

                position.plies_without_advancement = 0;

                check_piece_crowning(board, move.capture.destination_indices[move.capture.destination_indices_size - 1]);

                break;
        }

        position.player = game::opponent(position.player);
        position.plies++;
    }

    void play_move(search::SearchNode& node, const game::Move& move) {
        switch (move.type) {
            case game::MoveType::Normal:
                assert(node.board[move.normal.source_index] != game::Square::None);
                assert(node.board[move.normal.destination_index] == game::Square::None);

                std::swap(node.board[move.normal.source_index], node.board[move.normal.destination_index]);

                if (!game::is_king_piece(node.board[move.normal.destination_index])) {
                    node.plies_without_advancement = 0;
                    node.previous = nullptr;
                } else {
                    node.plies_without_advancement++;
                }

                check_piece_crowning(node.board, move.normal.destination_index);

                break;
            case game::MoveType::Capture:
                assert(node.board[move.capture.source_index] != game::Square::None);
                assert(
                    node.board[move.capture.destination_indices[move.capture.destination_indices_size - 1]] == game::Square::None ||
                    move.capture.source_index == move.capture.destination_indices[move.capture.destination_indices_size - 1]
                );

                remove_jumped_pieces(node.board, move);
                std::swap(
                    node.board[move.capture.source_index],
                    node.board[move.capture.destination_indices[move.capture.destination_indices_size - 1]]
                );

                node.plies_without_advancement = 0;
                node.previous = nullptr;

                check_piece_crowning(node.board, move.capture.destination_indices[move.capture.destination_indices_size - 1]);

                break;
        }

        node.player = game::opponent(node.player);
        node.plies++;
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
