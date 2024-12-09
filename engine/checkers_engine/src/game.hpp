#pragma once

#include <array>
#include <string>
#include <type_traits>
#include <cstdint>

namespace search {
    struct SearchNode;
}

namespace game {
    inline constexpr int NULL_INDEX {-1};  // Indices may be in the range [0, 31] or [1, 32]

    enum class Square : unsigned char {
        None      = 0b000u,
        Black     = 0b001u,
        BlackKing = 0b101u,
        White     = 0b010u,
        WhiteKing = 0b110u
    };

    enum class Player : unsigned char {
        Black = 0b01u,
        White = 0b10u
    };

    using Board = std::array<Square, 32>;

    struct Position {
        Board board {};
        Player player {Player::Black};  // Next player to move
    };

    struct GamePosition {
        Board board {};
        Player player {Player::Black};  // Next player to move
        int plies_without_advancement {0};
    };

    enum class MoveType : int {
        Normal = 0,
        Capture = 1
    };

    struct Move {
        // Indices are always in the range [0, 31]
        // Array destination_indices must be clean

        using DestinationIndices = std::array<int, 9>;

        constexpr Move() noexcept = default;

        constexpr Move(int source_index, int destination_index) noexcept {
            value |= static_cast<std::uint64_t>(MoveType::Normal);
            value |= static_cast<std::uint64_t>(source_index) << 1;
            value |= static_cast<std::uint64_t>(destination_index) << 6;
            value |= static_cast<std::uint64_t>(1) << 51;
        }

        constexpr Move(int source_index, const DestinationIndices& destination_indices, int destination_indices_size) noexcept {
            value |= static_cast<std::uint64_t>(MoveType::Capture);
            value |= static_cast<std::uint64_t>(source_index) << 1;

            for (int i {0}; i < 9; i++) {
                value |= static_cast<std::uint64_t>(destination_indices[i]) << (i * 5 + 6);
            }

            value |= static_cast<std::uint64_t>(destination_indices_size) << 51;
        }

        constexpr MoveType type() const noexcept {
            return static_cast<MoveType>(value & 1ul);
        }

        constexpr int source_index() const noexcept {
            return static_cast<int>(value >> 1 & 0b11111ul);
        }

        constexpr int destination_index(int index = 0) const noexcept {
            return static_cast<int>(value >> (index * 5 + 6) & 0b11111ul);
        }

        constexpr int destination_indices_size() const noexcept {
            return static_cast<int>(value >> 51 & 0xFFul);
        }

        constexpr bool operator==(Move other) const noexcept {
            return value == other.value;
        }

        constexpr bool operator!=(Move other) const noexcept {
            return value != other.value;
        }

        std::uint64_t value {};
    };

    inline constexpr Move NULL_MOVE {};

    constexpr bool is_move_capture(Move move) noexcept {
        return move.type() == MoveType::Capture;
    }

    constexpr int _1_32_to_0_31(int index) noexcept {
        return index - 1;
    }

    constexpr int _0_31_to_1_32(int index) noexcept {
        return index + 1;
    }

    constexpr bool is_black_piece(Square square) noexcept {
        return static_cast<std::underlying_type_t<Square>>(square) & 1u << 0;
    }

    constexpr bool is_white_piece(Square square) noexcept {
        return static_cast<std::underlying_type_t<Square>>(square) & 1u << 1;
    }

    constexpr bool is_king_piece(Square square) noexcept {
        return static_cast<std::underlying_type_t<Square>>(square) & 1u << 2;
    }

    constexpr bool is_piece(Square square, Player player) noexcept {
        return static_cast<std::underlying_type_t<Square>>(square) & static_cast<std::underlying_type_t<Player>>(player);
    }

    constexpr bool is_move_advancement(const Board& board, Move move) noexcept {
        // Must be called right before the move has been played on the board

        if (move.type() == MoveType::Normal) {
            return !is_king_piece(board[move.source_index()]);
        } else {
            return true;
        }
    }

    constexpr Player opponent(Player player) noexcept {
        if (player == Player::Black) {
            return Player::White;
        } else {
            return Player::Black;
        }
    }

    Move parse_move_string(const std::string& move_string);
    void set_position(GamePosition& position, const std::string& fen_string);
    void play_move(GamePosition& position, const std::string& move_string);
    void play_move(GamePosition& position, Move move) noexcept;
    void play_move(search::SearchNode& node, Move move) noexcept;
}
