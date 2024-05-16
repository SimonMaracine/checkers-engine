#include <gtest/gtest.h>
#include <moves.hpp>
#include <game.hpp>

static unsigned int count_moves(unsigned int depth, game::Position& position) {
    unsigned int total_moves {0u};

    if (depth == 0u) {
        return 1u;
    }

    const auto moves {moves::generate_moves(position.board, position.player)};

    for (const game::Move& move : moves) {
        auto new_position {position};
        moves::play_move(new_position, move);

        total_moves += count_moves(depth - 1u, new_position);
    }

    return total_moves;
}

static unsigned int test_moves_from_position(unsigned int depth, const char* fen_string) {
    game::Position position;
    game::set_position(position, fen_string);

    return count_moves(depth, position);
}

TEST(move_generation, move_generation_start) {
    const char* fen_string {"B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32"};

    ASSERT_EQ(test_moves_from_position(1u, fen_string), 7u);
    ASSERT_EQ(test_moves_from_position(2u, fen_string), 49u);
}
