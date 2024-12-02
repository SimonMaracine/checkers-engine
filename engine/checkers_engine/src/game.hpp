#pragma once

#include <array>
#include <string>

namespace search {
    struct SearchNode;
}

namespace game {
    using Idx = signed char;  // Indices may be in the range [0, 31] or [1, 32]

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

    using Board = std::array<Square, 32>;

    struct Position {
        Board board {};
        Player player {Player::Black};  // Next player to move
        unsigned int plies {0};  // TODO might not be needed
        unsigned int plies_without_advancement {0};
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
                Idx destination_indices[9];
                unsigned char destination_indices_size;
            } capture;
        };

        MoveType type {};

        bool operator==(const Move& other) const;
    };

    void set_position(Position& position, const std::string& fen_string);
    void play_move(Position& position, const std::string& move_string);
    void play_move(Position& position, const Move& move);
    void play_move(search::SearchNode& node, const Move& move);
    Move parse_move_string(const std::string& move_string);
    Player opponent(Player player);
    bool is_move_advancement(const Board& board, const Move& move);

    constexpr Idx _1_32_to_0_31(const Idx index) {
        return index - 1;
    }

    constexpr Idx _0_31_to_1_32(const Idx index) {
        return index + 1;
    }

    constexpr bool is_black_piece(const Square square) {
        return static_cast<bool>(static_cast<unsigned char>(square) & 1u << 0);
    }

    constexpr bool is_white_piece(const Square square) {
        return static_cast<bool>(static_cast<unsigned char>(square) & 1u << 1);
    }

    constexpr bool is_king_piece(const Square square) {
        return static_cast<bool>(static_cast<unsigned char>(square) & 1u << 2);
    }

    constexpr bool is_piece(const Square square, const Player player) {
        return static_cast<bool>(static_cast<unsigned char>(square) & static_cast<unsigned char>(player));
    }
}
