#include "zobrist.hpp"

#include <random>
#include <cstring>

namespace zobrist {
    Zobrist instance;

    void Zobrist::initialize() {
        // Numbers must be initialized to 0, because some of them will not get a random value
        std::memset(m_random_numbers, 0, COLORS * PIECES * SQUARES * sizeof(std::uint64_t));

        std::mt19937_64 rng;
        std::uniform_int_distribution<std::mt19937_64::result_type> distribution;

        for (int cl {0}; cl < COLORS; cl++) {
            if (cl == 0) {
                continue;
            }

            for (int pc {0}; pc < PIECES; pc++) {
                for (int sq {0}; sq < SQUARES; sq++) {
                    m_random_numbers[cl][pc][sq] = distribution(rng);
                }
            }
        }

        m_random_number = distribution(rng);
    }
}
