#pragma once

#include <unordered_map>
#include <cstdint>
#include <optional>

#include "game.hpp"
#include "evaluation.hpp"

// https://www.boost.org/doc/libs/1_86_0/libs/container_hash/doc/html/hash.html
// https://en.cppreference.com/w/cpp/container/unordered_map
// https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

namespace transposition_table {
    struct Position {
        game::Board board {};
        game::Player player {};

        bool operator==(const Position& other) const noexcept {
            return board == other.board && player == other.player;
        }
    };
}

template<>
struct std::hash<transposition_table::Position> {
    template<typename T>
    static void hash_combine(std::size_t& seed, const T& value) noexcept {
        seed ^= std::hash<T>()(value) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
    }

    std::size_t operator()(const transposition_table::Position& position) const noexcept {
        std::uint64_t value0 {};
        std::uint64_t value1 {};

        for (std::size_t i {0}; i < 16; i++) {
            value0 |= static_cast<std::uint64_t>(position.board[i]) << (i * 3);
        }

        for (std::size_t i {16}; i < 32; i++) {
            value1 |= static_cast<std::uint64_t>(position.board[i]) << ((i - 16) * 3);
        }

        value1 |= static_cast<std::uint64_t>(position.player) << 46;

        std::size_t result {0};
        hash_combine(result, value0);
        hash_combine(result, value1);

        return result;
    }
};

namespace transposition_table {
    struct TableEntry {
        unsigned int depth {};
        evaluation::Eval eval {};
        game::Move move {};
    };

    class TranspositionTable {
    public:
        void store(const Position& position, unsigned int depth, evaluation::Eval eval, const game::Move& move);
        const TableEntry* retrieve(const Position& position, unsigned int depth) const;
        void clear();
        float load_factor() const;
    private:
        std::unordered_map<Position, TableEntry> m_table;
    };
}
