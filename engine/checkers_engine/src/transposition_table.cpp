#include "transposition_table.hpp"

namespace transposition_table {
    void TranspositionTable::store(const Position& position, unsigned int depth, evaluation::Eval eval, const game::Move& move) {
        m_table[position] = { depth, eval, move };
    }

    const TableEntry* TranspositionTable::retrieve(const Position& position, unsigned int depth) const {
        const auto iter {m_table.find(position)};

        if (iter == m_table.cend()) {
            return nullptr;
        }

        if (iter->second.depth >= depth) {
            return &iter->second;
        }

        return nullptr;
    }

    void TranspositionTable::clear() {
        m_table.clear();
    }

    float TranspositionTable::load_factor() const {
        return m_table.load_factor();
    }
}
