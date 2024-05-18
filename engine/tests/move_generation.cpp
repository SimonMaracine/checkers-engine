#include <iostream>
#include <cstddef>
#include <gtest/gtest.h>
#include <moves.hpp>
#include <game.hpp>

static std::uint64_t count_moves(unsigned int depth, game::Position& position) {
    std::uint64_t total_moves {0u};

    if (depth == 0u) {
        return 1ul;
    }

    const auto moves {moves::generate_moves(position.board, position.player)};

    for (const game::Move& move : moves) {
        auto new_position {position};
        moves::play_move(new_position, move);

        total_moves += count_moves(depth - 1u, new_position);
    }

    if (position.plies == 1u) {
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
    ASSERT_EQ(test_moves_from_position(1u, fen_string), 7u);
    ASSERT_EQ(test_moves_from_position(2u, fen_string), 49u);
    ASSERT_EQ(test_moves_from_position(3u, fen_string), 302u);
    ASSERT_EQ(test_moves_from_position(4u, fen_string), 1469u);
    ASSERT_EQ(test_moves_from_position(5u, fen_string), 7361u);
    ASSERT_EQ(test_moves_from_position(6u, fen_string), 36768u);
    ASSERT_EQ(test_moves_from_position(7u, fen_string), 179740u);
    ASSERT_EQ(test_moves_from_position(8u, fen_string), 845931u);
    ASSERT_EQ(test_moves_from_position(9u, fen_string), 3963680u);
    ASSERT_EQ(test_moves_from_position(10u, fen_string), 18391564u);
    ASSERT_EQ(test_moves_from_position(11u, fen_string), 85242128u);
    ASSERT_EQ(test_moves_from_position(12u, fen_string), 388623673u);
    ASSERT_EQ(test_moves_from_position(13u, fen_string), 1766623630u);
#elif 1
    ASSERT_EQ(test_moves_from_position(14u, fen_string), 7978439499ul);
#else
    ASSERT_EQ(test_moves_from_position(15u, fen_string), 36263167175ul);
#endif
}
