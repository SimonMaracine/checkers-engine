#pragma once

#include <cstdint>
#include <type_traits>

#include "game.hpp"

namespace zobrist {
    class Zobrist {
    public:
        void initialize();

        std::uint64_t hash(const game::Position& position) const noexcept {
            std::uint64_t result {0};

            for (int i {0}; i < 32; i++) {
                if (position.board[i] == game::Square::None) {
                    continue;
                }

                const auto square {static_cast<std::underlying_type_t<game::Square>>(position.board[i])};
                result ^= m_random_numbers[square >> 2 & 1u][square >> 1 & 1u][i];
            }

            if (position.player == game::Player::White) {
                result ^= m_random_number;
            }

            return result;
        }

        std::uint64_t hash_mod(game::Square square, int index) const noexcept {
            const auto square_ {static_cast<std::underlying_type_t<game::Square>>(square)};
            return m_random_numbers[square_ >> 2 & 1u][square_ >> 1 & 1u][index];
        }

        std::uint64_t hash_mod() const noexcept {
            return m_random_number;
        }
    private:
        static constexpr int PIECES {2};
        static constexpr int COLORS {2};
        static constexpr int SQUARES {32};

        std::uint64_t m_random_numbers[PIECES][COLORS][SQUARES] {};
        std::uint64_t m_random_number {};
    };

    // Yes, global
    extern Zobrist instance;
}
