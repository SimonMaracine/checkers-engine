#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <random>

namespace opening {
    class OpeningBook {
    public:
        void initialize(std::unordered_map<std::string, std::vector<std::string>>&& data);
        std::optional<std::string> lookup(const std::string& position);
    private:
        std::unordered_map<std::string, std::vector<std::string>> m_data;
        std::mt19937_64 m_random;
    };
}
