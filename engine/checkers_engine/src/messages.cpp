#include "messages.hpp"

#include <iostream>
#include <mutex>
#include <cassert>

#include "utils.hpp"

namespace messages {
    static std::mutex g_mutex;

    static char get_square_char(game::Square square) noexcept {
        switch (square) {
            case game::Square::None:
                return ' ';
            case game::Square::Black:
                return 'b';
            case game::Square::BlackKing:
                return 'B';
            case game::Square::White:
                return 'w';
            case game::Square::WhiteKing:
                return 'W';
        }

        assert(false);
        utils::unreachable();
    }

    /*
        Using std::endl is crucial.

        Because multiple threads may print messages and because these are split between multiple << calls,
        every message function needs to be protected by a mutex.
    */

    void ready() {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "READY" << std::endl;
    }

    void bestmove(game::Move move) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "BESTMOVE ";

        if (move != game::NULL_MOVE) {
            std::cout << game::move_to_string(move);
        } else {
            std::cout << "none";
        }

        std::cout << std::endl;
    }

    void parameters(const std::unordered_map<std::string, parameters::Parameter>& parameters) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "PARAMETERS";

        for (const auto& [name, _] : parameters) {
            std::cout << ' ' << name;
        }

        std::cout << std::endl;
    }

    void parameter(const std::string& name, const parameters::Parameter& value) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "PARAMETER " << name;

        switch (value.index()) {  // Too bad that I can't just use typeid
            case 0:
                std::cout << " int " << std::get<0>(value);
                break;
            case 1:
                std::cout << " float " << std::get<1>(value);
                break;
            case 2:
                std::cout << " bool " << std::get<2>(value);
                break;
            case 3:
                std::cout << " string " << std::get<3>(value);
                break;
            default:
                assert(false);
                break;
        }

        std::cout << std::endl;
    }

    void info(
        int nodes,
        int transpositions,
        int depth,
        evaluation::Eval eval,
        double time,
        const game::Move* pv_moves,
        int pv_size
    ) {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "INFO ";
        std::cout << "nodes " << nodes << ' ';
        std::cout << "transpositions " << transpositions << ' ';
        std::cout << "depth " << depth << ' ';
        std::cout << "eval " << eval << ' ';
        std::cout << "time " << time << ' ';
        std::cout << "pv";

        for (int i {0}; i < pv_size; i++) {
            std::cout << ' ' << game::move_to_string(pv_moves[i]);
        }

        std::cout << std::endl;
    }

    void name() {
        std::lock_guard<std::mutex> lock {g_mutex};

        std::cout << "NAME " << "chuck|18.0" << std::endl;
    }

    void board(const game::GamePosition& position) {
        std::lock_guard<std::mutex> lock {g_mutex};

        int pc {0};
        int k {0};

        std::cout << "+---+---+---+---+---+---+---+---+\n";

        for (int i {0}; i < 8; i++) {
            std::cout << "| ";

            for (int j {0}; j < 7; j++, k++) {
                if (k % 2 == 0) {
                    std::cout << "  | ";
                } else {
                    std::cout << get_square_char(position.board[pc++]);
                    std::cout << " | ";
                }
            }

            if (k % 2 == 0) {
                std::cout << ' ';
            } else {
                std::cout << get_square_char(position.board[pc++]);
            }
            std::cout << " |\n";

            std::cout << "+---+---+---+---+---+---+---+---+\n";
        }

        std::cout << std::endl;
    }
}
