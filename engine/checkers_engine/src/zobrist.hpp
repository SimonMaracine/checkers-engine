#pragma once

#include <cstdint>
#include <type_traits>

#include "game.hpp"

// https://web.archive.org/web/20071031100138/http://www.brucemo.com/compchess/programming/zobrist.htm
// https://www.chessprogramming.org/Zobrist_Hashing
// https://www.chessprogramming.org/Transposition_Table#KeyCollisions

namespace zobrist {
    class Zobrist {
    public:
        void initialize();

        std::uint64_t hash(const game::Position& position) const noexcept {
            std::uint64_t result {0};

            for (int i {0}; i < 32; i++) {
                const auto square {static_cast<std::underlying_type_t<game::Square>>(position.board[i])};
                result ^= m_random_numbers[square & 0b11u][square >> 2 & 1u][i];
            }

            if (position.player == game::Player::White) {
                result ^= m_random_number;
            }

            return result;
        }

        std::uint64_t hash_mod(game::Square square, int index) const noexcept {
            const auto square_ {static_cast<std::underlying_type_t<game::Square>>(square)};
            return m_random_numbers[square_ & 0b11u][square_ >> 2 & 1u][index];
        }

        std::uint64_t hash_mod() const noexcept {
            return m_random_number;
        }
    private:
        static constexpr int COLORS {3};
        static constexpr int PIECES {2};
        static constexpr int SQUARES {32};

        std::uint64_t m_random_numbers[COLORS][PIECES][SQUARES] {};
        std::uint64_t m_random_number {};
    };

    // Yes, global
    extern Zobrist instance;
}
