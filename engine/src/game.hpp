#pragma once

#include <array>

using Idx = signed char;

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
    Player player {};
    unsigned int plies_without_advancement {};
    // TODO other fields?
};

enum class MoveType : unsigned char {
    Normal,
    Capture
};

struct Move {
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
};
