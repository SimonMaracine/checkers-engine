#include "transposition_table.hpp"

namespace transposition_table {
    void TranspositionTable::store(
        const Position& position,
        int depth,
        NodeType node_type,
        evaluation::Eval eval,
        game::Move move
    ) {
        m_table[position] = { depth, node_type, eval, move };
    }

    std::pair<evaluation::Eval, game::Move> TranspositionTable::retrieve(
        const Position& position,
        int depth,
        evaluation::Eval alpha,
        evaluation::Eval beta
    ) const {
        const auto iter {m_table.find(position)};

        if (iter == m_table.cend()) {
            return std::make_pair(evaluation::INVALID, game::NULL_MOVE);
        }

        const TableEntry& entry {iter->second};

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

    void TranspositionTable::clear() {
        m_table.clear();
    }
}
