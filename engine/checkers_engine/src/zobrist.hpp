#pragma once

#include <cstdint>
#include <type_traits>

#include "game.hpp"

namespace zobrist {
    class Zobrist {
    public:
        Zobrist();

        constexpr std::uint64_t hash(const game::Position& position) const noexcept {
            std::uint64_t result {0};

            for (int i {0}; i < 32; i++) {
                if (position.board[i] == game::Square::None) {
                    continue;
                }

                const auto square {static_cast<std::underlying_type_t<game::Square>>(position.board[i])};
                result ^= m_random_numbers[square & 1u << 2][square & 1u << 1][i];
            }

            if (position.player == game::Player::White) {
                result ^= m_white_random_number;
            }

            return result;
        }
    private:
        static constexpr int PIECES {2};
        static constexpr int COLORS {2};
        static constexpr int SQUARES {32};

        std::uint64_t m_random_numbers[PIECES][COLORS][SQUARES] {};
        std::uint64_t m_white_random_number {};
    };
}
