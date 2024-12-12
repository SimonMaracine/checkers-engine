#pragma once

#include <cstddef>

#include "game.hpp"
#include "evaluation.hpp"

// https://www.chessprogramming.org/Node_Types
// https://stackoverflow.com/questions/18439520/is-there-a-128-bit-integer-in-c
// https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm
// https://www.chessprogramming.org/Transposition_Table#KeyCollisions

namespace transposition_table {
    using Key = game::PositionKey;
    using Signature = game::PositionSignature;

    enum class Flag : int {
        Exact,
        Beta,
        Alpha
    };

    struct TableEntry {
        // signature and depth must be initialized to 0
        Signature signature {0};
        game::Move move {};
        int depth {0};
        Flag flag {};
        evaluation::Eval eval {};
    };

    using TableEntryResult = std::pair<evaluation::Eval, game::Move>;

    class TranspositionTable {
    public:
        TranspositionTable() noexcept = default;
        ~TranspositionTable() noexcept;

        TranspositionTable(const TranspositionTable&) = delete;
        TranspositionTable& operator=(const TranspositionTable&) = delete;
        TranspositionTable(TranspositionTable&&) = delete;
        TranspositionTable& operator=(TranspositionTable&&) = delete;

        void allocate(std::size_t size_bytes);

        void store(const game::Position& position, int depth, Flag flag, evaluation::Eval eval, game::Move move) noexcept;
        void store(Key key, Signature signature, int depth, Flag flag, evaluation::Eval eval, game::Move move) noexcept;
        TableEntryResult load(const game::Position& position, int depth, evaluation::Eval alpha, evaluation::Eval beta) const noexcept;
        TableEntryResult load(Key key, Signature signature, int depth, evaluation::Eval alpha, evaluation::Eval beta) const noexcept;

        void clear() noexcept;

        std::size_t size() const noexcept {
            return m_size;
        }
    private:
        TableEntry* m_entries {nullptr};
        std::size_t m_size {0};
    };

    constexpr std::size_t mib_to_bytes(std::size_t mib) noexcept {
        return mib * 1024 * 1024;
    }
}
