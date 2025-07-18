#include <iostream>
#include <cstdint>

#include <gtest/gtest.h>
#include <moves.hpp>
#include <game.hpp>

static std::uint64_t count_moves(int depth, const game::GamePosition& position) {
    std::uint64_t total_moves {0};

    if (depth == 0) {
        return 1;
    }

    const auto moves {moves::generate_moves(position)};

    for (const game::Move move : moves) {
        game::GamePosition new_position {position};
        game::play_move(new_position, move);

        total_moves += count_moves(depth - 1, new_position);
    }

    return total_moves;
}

static std::uint64_t test_moves_from_position(int depth, const char* fen_string) {
    game::GamePosition position;
    game::set_position(position, fen_string);

    return count_moves(depth, position);
}

static const char* FEN_STRING {"B:B1,2,3,4,5,6,7,8,9,10,11,12:W21,22,23,24,25,26,27,28,29,30,31,32"};

// https://oeis.org/A133046/list

#if 1
    TEST(move_generation, move_generation_start_1) {
        ASSERT_EQ(test_moves_from_position(1, FEN_STRING), 7);
    }

    TEST(move_generation, move_generation_start_2) {
        ASSERT_EQ(test_moves_from_position(2, FEN_STRING), 49);
    }

    TEST(move_generation, move_generation_start_3) {
        ASSERT_EQ(test_moves_from_position(3, FEN_STRING), 302);
    }

    TEST(move_generation, move_generation_start_4) {
        ASSERT_EQ(test_moves_from_position(4, FEN_STRING), 1469);
    }

    TEST(move_generation, move_generation_start_5) {
        ASSERT_EQ(test_moves_from_position(5, FEN_STRING), 7361);
    }

    TEST(move_generation, move_generation_start_6) {
        ASSERT_EQ(test_moves_from_position(6, FEN_STRING), 36'768);
    }

    TEST(move_generation, move_generation_start_7) {
        ASSERT_EQ(test_moves_from_position(7, FEN_STRING), 179'740);
    }

    TEST(move_generation, move_generation_start_8) {
        ASSERT_EQ(test_moves_from_position(8, FEN_STRING), 845'931);
    }

    TEST(move_generation, move_generation_start_9) {
        ASSERT_EQ(test_moves_from_position(9, FEN_STRING), 3'963'680);
    }

    TEST(move_generation, move_generation_start_10) {
        ASSERT_EQ(test_moves_from_position(10, FEN_STRING), 18'391'564);
    }

    TEST(move_generation, move_generation_start_11) {
        ASSERT_EQ(test_moves_from_position(11, FEN_STRING), 85'242'128);
    }

    TEST(move_generation, move_generation_start_12) {
        ASSERT_EQ(test_moves_from_position(12, FEN_STRING), 388'623'673);
    }

    TEST(move_generation, move_generation_start_13) {
        ASSERT_EQ(test_moves_from_position(13, FEN_STRING), 1'766'623'630);
    }
#elif 0
    TEST(move_generation, move_generation_start_14) {
        ASSERT_EQ(test_moves_from_position(14, FEN_STRING), 7'978'439'499ul);
    }
#else
    TEST(move_generation, move_generation_start_15) {
        ASSERT_EQ(test_moves_from_position(15, FEN_STRING), 36'263'167'175ul);
    }
#endif
