#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>

#include "game.hpp"
#include "evaluation.hpp"
#include "zobrist.hpp"

// https://www.chessprogramming.org/Node_Types
// https://stackoverflow.com/questions/18439520/is-there-a-128-bit-integer-in-c
// https://web.archive.org/web/20071031100138/http://www.brucemo.com/compchess/programming/zobrist.htm
// https://www.chessprogramming.org/Zobrist_Hashing
// https://www.chessprogramming.org/Transposition_Table#KeyCollisions

namespace transposition_table {
    using Signature = __uint128_t;
    using KeyType = std::uint64_t;

    enum class NodeType {
        Pv,
        Cut,
        All
    };

    struct TableEntry {
        // These two are important to be initialized to 0
        Signature signature {0};
        int depth {0};

        NodeType node_type {};
        evaluation::Eval eval {};
        game::Move move {};
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
        void store(const game::Position& position, int depth, NodeType node_type, evaluation::Eval eval, game::Move move) noexcept;
        TableEntryResult retrieve(const game::Position& position, int depth, evaluation::Eval alpha, evaluation::Eval beta) const noexcept;
        void clear() noexcept;
    private:
        static constexpr Signature signature(const game::Position& position) noexcept {
            Signature result {0};

            for (int i {0}; i < 32; i++) {
                result |= static_cast<Signature>(position.board[i]) << (i * 3);
            }

            result |= static_cast<Signature>(position.player) << 96;

            return result;
        }

        TableEntry* m_entries {nullptr};
        std::size_t m_size {0};
        zobrist::Zobrist m_zobrist;
    };

    constexpr std::size_t mib_to_bytes(std::size_t mib) noexcept {
        return mib * 1024 * 1024;
    }
}
