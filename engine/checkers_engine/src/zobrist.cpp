#include "zobrist.hpp"

#include <random>

namespace zobrist {
    Zobrist::Zobrist() {
        std::mt19937_64 rng;
        std::uniform_int_distribution<std::mt19937_64::result_type> distribution;

        for (int pc {0}; pc < PIECES; pc++) {
            for (int cl {0}; cl < COLORS; cl++) {
                for (int sq {0}; sq < SQUARES; sq++) {
                    m_random_numbers[pc][cl][sq] = distribution(rng);
                }
            }
        }

        m_white_random_number = distribution(rng);
    }
}
