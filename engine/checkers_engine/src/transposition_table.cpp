#include "transposition_table.hpp"

#include <cassert>

#include "zobrist.hpp"

namespace transposition_table {
    TranspositionTable::~TranspositionTable() noexcept {
        delete[] m_entries;
    }

    void TranspositionTable::allocate(std::size_t size_bytes) {
        delete[] m_entries;

        m_size = size_bytes / sizeof(TableEntry);
        m_entries = new TableEntry[m_size];
    }

    void TranspositionTable::store(const game::Position& position, int depth, Flag flag, evaluation::Eval eval, game::Move move) noexcept {
        store(zobrist::instance.hash(position), game::signature(position), depth, flag, eval, move);
    }

    void TranspositionTable::store(Key key, Signature signature, int depth, Flag flag, evaluation::Eval eval, game::Move move) noexcept {
        assert(m_size != 0);

        TableEntry& entry {m_entries[key % m_size]};

        // Scheme "replace if same depth or deeper"

        if (entry.depth <= depth) {
            entry.signature = signature;
            entry.move = move;
            entry.depth = depth;
            entry.flag = flag;
            entry.eval = eval;
        }
    }

    TableEntryResult TranspositionTable::load(const game::Position& position, int depth, evaluation::Eval alpha, evaluation::Eval beta) const noexcept {
        return load(zobrist::instance.hash(position), game::signature(position), depth, alpha, beta);
    }

    TableEntryResult TranspositionTable::load(Key key, Signature signature, int depth, evaluation::Eval alpha, evaluation::Eval beta) const noexcept {
        assert(m_size != 0);

        const TableEntry& entry {m_entries[key % m_size]};

        if (entry.signature != signature) {
            return std::make_pair(evaluation::UNKNOWN, game::NULL_MOVE);
        }

        if (entry.depth >= depth) {
            if (entry.flag == Flag::Exact) {
                return std::make_pair(entry.eval, game::NULL_MOVE);
            }

            if (entry.flag == Flag::Alpha && entry.eval <= alpha) {
                return std::make_pair(alpha, game::NULL_MOVE);
            }

            if (entry.flag == Flag::Beta && entry.eval >= beta) {
                return std::make_pair(beta, game::NULL_MOVE);
            }
        }

        // Also return the hash move to be used in reordering
        return std::make_pair(evaluation::UNKNOWN, entry.move);
    }

    void TranspositionTable::clear() noexcept {
        for (std::size_t i {0}; i < m_size; i++) {
            m_entries[i] = {};
        }
    }
}
