#include "transposition_table.hpp"

namespace transposition_table {
    TranspositionTable::~TranspositionTable() noexcept {
        delete[] m_entries;
    }

    void TranspositionTable::allocate(std::size_t size_bytes) {
        delete[] m_entries;

        m_size = size_bytes / sizeof(TableEntry);
        m_entries = new TableEntry[m_size];
    }

    void TranspositionTable::store(const game::Position& position, int depth, NodeType node_type, evaluation::Eval eval, game::Move move) noexcept {
        TableEntry& entry {m_entries[m_zobrist.hash(position) % m_size]};

        // Scheme "replace if same depth or deeper"

        if (entry.depth <= depth) {
            entry.signature = signature(position);
            entry.depth = depth;
            entry.node_type = node_type;
            entry.eval = eval;
            entry.move = move;
        }
    }

    TableEntryResult TranspositionTable::retrieve(const game::Position& position, int depth, evaluation::Eval alpha, evaluation::Eval beta) const noexcept {
        const TableEntry& entry {m_entries[m_zobrist.hash(position) % m_size]};

        if (entry.signature != signature(position)) {
            return std::make_pair(evaluation::INVALID, game::NULL_MOVE);
        }

        if (entry.depth >= depth) {
            if (entry.node_type == NodeType::Pv) {
                return std::make_pair(entry.eval, entry.move);
            }

            if (entry.node_type == NodeType::All && entry.eval <= alpha) {
                return std::make_pair(alpha, entry.move);
            }

            if (entry.node_type == NodeType::Cut && entry.eval >= beta) {
                return std::make_pair(beta, entry.move);
            }
        }

        return std::make_pair(evaluation::INVALID, game::NULL_MOVE);
    }

    void TranspositionTable::clear() noexcept {
        for (std::size_t i {0}; i < m_size; i++) {
            m_entries[i] = {};
        }
    }
}
