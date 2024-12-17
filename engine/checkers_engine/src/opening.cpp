#include "opening.hpp"

#include <utility>

namespace opening {
    void OpeningBook::initialize(std::unordered_map<std::string, std::vector<std::string>>&& data) {
        m_data = std::move(data);
        m_random = std::mt19937_64(std::random_device()());
    }

    std::optional<std::string> OpeningBook::lookup(const std::string& position) {
        if (const auto iter {m_data.find(position)}; iter != m_data.cend()) {
            const auto& moves {iter->second};

            if (moves.empty()) {
                return std::nullopt;
            }

            std::uniform_int_distribution<std::mt19937_64::result_type> distribution {0, moves.size() - 1};

            return std::make_optional(moves.at(distribution(m_random)));
        }

        return std::nullopt;
    }
}
