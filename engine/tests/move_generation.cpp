#include <iostream>
#include <cstddef>
#include <gtest/gtest.h>
#include <moves.hpp>
#include <game.hpp>

static std::uint64_t count_moves(unsigned int depth, game::Position& position) {
    std::uint64_t total_moves {0};

    if (depth == 0) {
        return 1;
    }

    const auto moves {moves::generate_moves(position.board, position.player)};

    for (const game::Move& move : moves) {
        auto new_position {position};
        moves::play_move(new_position, move);

        total_moves += count_moves(depth - 1, new_position);
    }

    if (position.plies == 1) {
        std::cout << total_moves << '\n';
    }

    return total_moves;
}

static std::uint64_t test_moves_from_position(unsigned int depth, const char* fen_string) {
    game::Position position;
    game::set_position(position, fen_string);

    return count_moves(depth, position);
}

// https://oeis.org/A133046/list

TEST(move_generation, move_generation_start) {
    const char* fen_string {"B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32"};

#if 0
    ASSERT_EQ(test_moves_from_position(1, fen_string), 7);
    ASSERT_EQ(test_moves_from_position(2, fen_string), 49);
    ASSERT_EQ(test_moves_from_position(3, fen_string), 302);
    ASSERT_EQ(test_moves_from_position(4, fen_string), 1469);
    ASSERT_EQ(test_moves_from_position(5, fen_string), 7361);
    ASSERT_EQ(test_moves_from_position(6, fen_string), 36768);
    ASSERT_EQ(test_moves_from_position(7, fen_string), 179740);
    ASSERT_EQ(test_moves_from_position(8, fen_string), 845931);
    ASSERT_EQ(test_moves_from_position(9, fen_string), 3963680);
    ASSERT_EQ(test_moves_from_position(10, fen_string), 18391564);
    ASSERT_EQ(test_moves_from_position(11, fen_string), 85242128);
    ASSERT_EQ(test_moves_from_position(12, fen_string), 388623673);
    ASSERT_EQ(test_moves_from_position(13, fen_string), 1766623630);
#elif 1
    ASSERT_EQ(test_moves_from_position(14, fen_string), 7978439499ul);
#else
    ASSERT_EQ(test_moves_from_position(15, fen_string), 36263167175ul);
#endif
}
