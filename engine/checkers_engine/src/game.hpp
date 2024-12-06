#pragma once

#include <array>
#include <string>
#include <cstdint>

namespace search {
    struct SearchNode;
}

namespace game {
    inline constexpr int NULL_INDEX {-1};  // Indices may be in the range [0, 31] or [1, 32]

    enum class Square : unsigned int {
        None      = 0b000u,
        Black     = 0b001u,
        BlackKing = 0b101u,
        White     = 0b010u,
        WhiteKing = 0b110u
    };

    enum class Player : unsigned int {
        Black = 0b01u,
        White = 0b10u
    };

    using Board = std::array<Square, 32>;

    struct Position {
        Board board {};
        Player player {Player::Black};  // Next player to move
        int plies_without_advancement {0};
    };

    enum class MoveType : unsigned int {
        Normal = 0,
        Capture = 1
    };

    struct Move {
        // Indices are always in the range [0, 31]

        // destination_indices must be a clean array

        using DestinationIndices = std::array<int, 9>;

        constexpr Move() = default;

        constexpr Move(int source_index, int destination_index) {
            value |= static_cast<std::uint64_t>(MoveType::Normal);
            value |= static_cast<std::uint64_t>(source_index) << 1;
            value |= static_cast<std::uint64_t>(destination_index) << 6;
            value |= static_cast<std::uint64_t>(1) << 51;
        }

        constexpr Move(int source_index, const DestinationIndices& destination_indices, int destination_indices_size) {
            value |= static_cast<std::uint64_t>(MoveType::Capture);
            value |= static_cast<std::uint64_t>(source_index) << 1;

            for (int i {0}; i < 9; i++) {
                value |= static_cast<std::uint64_t>(destination_indices[i]) << (i * 5 + 6);
            }

            value |= static_cast<std::uint64_t>(destination_indices_size) << 51;
        }

        constexpr MoveType type() const {
            return static_cast<MoveType>(value & 1ul);
        }

        constexpr int source_index() const {
            return static_cast<int>(value >> 1 & 0b11111ul);
        }

        constexpr int destination_index(int index = 0) const {
            return static_cast<int>(value >> (index * 5 + 6) & 0b11111ul);
        }

        constexpr int destination_indices_size() const {
            return static_cast<int>(value >> 51 & 0xFFul);
        }

        constexpr bool operator==(Move other) const {
            return value == other.value;
        }

        constexpr bool operator!=(Move other) const {
            return value != other.value;
        }

        std::uint64_t value {};
    };

    inline constexpr Move NULL_MOVE {};

    void set_position(Position& position, const std::string& fen_string);
    void play_move(Position& position, const std::string& move_string);
    void play_move(Position& position, Move move);
    void play_move(search::SearchNode& node, Move move);
    Move parse_move_string(const std::string& move_string);
    Player opponent(Player player);
    bool is_move_advancement(const Board& board, Move move);
    bool is_move_capture(Move move);

    constexpr int _1_32_to_0_31(int index) {
        return index - 1;
    }

    constexpr int _0_31_to_1_32(int index) {
        return index + 1;
    }

    constexpr bool is_black_piece(Square square) {
        return static_cast<unsigned int>(square) & 1u << 0;
    }

    constexpr bool is_white_piece(Square square) {
        return static_cast<unsigned int>(square) & 1u << 1;
    }

    constexpr bool is_king_piece(Square square) {
        return static_cast<unsigned int>(square) & 1u << 2;
    }

    constexpr bool is_piece(Square square, Player player) {
        return static_cast<unsigned int>(square) & static_cast<unsigned int>(player);
    }
}
