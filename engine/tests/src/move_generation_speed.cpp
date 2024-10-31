#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>

#include <moves.hpp>
#include <game.hpp>

static void generate_moves(unsigned int depth, const game::Position& position) {
    if (depth == 0) {
        return;
    }

    const auto moves {moves::generate_moves(position.board, position.player)};

    for (const game::Move& move : moves) {
        game::Position new_position {position};
        moves::play_move(new_position, move);

        generate_moves(depth - 1, new_position);
    }
}

static double test_moves_from_position(unsigned int depth, const char* fen_string) {
    game::Position position;
    game::set_position(position, fen_string);

    const auto start {std::chrono::high_resolution_clock::now()};
    generate_moves(depth, position);
    const auto end {std::chrono::high_resolution_clock::now()};

    return std::chrono::duration<double>(end - start).count();
}

int main() {
    const char* fen_string {"B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32"};

    std::vector<double> times;

    for (unsigned int i {1}; i <= 11; i++) {
        const auto time {test_moves_from_position(i, fen_string)};

        times.push_back(time);
        std::cout << time << '\n';
    }

    std::ofstream stream {"move_generation_time_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".txt"};

    if (!stream.is_open()) {
        std::cerr << "Could not write times to file\n";
        return 1;
    }

    for (const auto time : times) {
        stream << time << '\n';
    }

    if (stream.fail()) {
        std::cerr << "Could not write times to file\n";
        return 1;
    }

    return 0;
}
