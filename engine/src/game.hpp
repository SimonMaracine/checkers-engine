#pragma once

#include <array>
#include <string>

namespace search {
    struct SearchNode;
}

namespace game {
    using Idx = signed char;  // Indices can be in the range [0, 31] or [1, 32]

    inline constexpr Idx NULL_INDEX {-1};

    enum class Square : unsigned char {
        None      = 0b0000u,
        Black     = 0b0001u,
        BlackKing = 0b0101u,
        White     = 0b0010u,
        WhiteKing = 0b0110u
    };

    enum class Player : unsigned char {
        Black = 0b0001u,
        White = 0b0010u
    };

    using Board = std::array<Square, 32u>;

    struct FenPosition {
        Board board {};
        Player player {Player::Black};
    };

    struct Position {
        FenPosition position;
        unsigned int plies {0u};
        unsigned int plies_without_advancement {0u};
    };

    enum class MoveType : unsigned char {
        Normal,
        Capture
    };

    struct Move {
        // Always in the range [0, 31]
        union {
            struct {
                Idx source_index;
                Idx destination_index;
            } normal;

            struct {
                Idx source_index;
                Idx destination_indices[9u];
                unsigned char destination_indices_size;
            } capture;
        };

        MoveType type {};
    };

    void set_position(FenPosition& position, const std::string& fen_string);
    void make_move(Position& position, const std::string& move_string);
    Player opponent(Player player);
    bool is_game_over(const search::SearchNode& node);

    constexpr Idx to_0_31(Idx index) {
        return index - 1;
    }

    constexpr Idx to_1_32(Idx index) {
        return index + 1;
    }

    constexpr bool is_black_piece(Square square) {
        return static_cast<bool>(static_cast<unsigned char>(square) & 1u << 0);
    }

    constexpr bool is_white_piece(Square square) {
        return static_cast<bool>(static_cast<unsigned char>(square) & 1u << 1);
    }
}
