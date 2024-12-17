#pragma once

#include <cstdint>

class Uint128t {
public:
	constexpr Uint128t() noexcept = default;

	constexpr Uint128t(std::uint8_t n) noexcept
		: m_lower(n) {}

	constexpr Uint128t(std::uint16_t n) noexcept
		: m_lower(n) {}

	constexpr Uint128t(std::uint32_t n) noexcept
		: m_lower(n) {}

	constexpr Uint128t(std::uint64_t n) noexcept
		: m_lower(n) {}

	template<typename T>
	constexpr Uint128t(T n) noexcept
		: m_lower(static_cast<std::uint64_t>(n)) {}

	constexpr bool operator==(const Uint128t& other) const noexcept {
		return m_lower == other.m_lower && m_upper == other.m_upper;
	}

	constexpr bool operator!=(const Uint128t& other) const noexcept {
		return !operator==(other);
	}

	constexpr Uint128t operator<<(int n) const noexcept {
		const std::uint64_t mask_lower {pow(2, n) << (64 - n)};
		const std::uint64_t migration_bits {(m_lower & mask_lower) >> (64 - n)};

		Uint128t result;
		result.m_lower = m_lower << n;
		result.m_upper = m_upper << n;
		result.m_upper |= migration_bits;

		return result;
	}

	constexpr Uint128t operator~() noexcept {
		Uint128t result;
		result.m_lower = ~m_lower;
		result.m_upper = ~m_upper;

		return result;
	}

	constexpr Uint128t operator&(const Uint128t& other) const noexcept {
		Uint128t result;
		result.m_lower = m_lower & other.m_lower;
		result.m_upper = m_upper & other.m_upper;

		return result;
	}

	constexpr Uint128t& operator&=(const Uint128t& other) noexcept {
		m_lower &= other.m_lower;
		m_upper &= other.m_upper;

		return *this;
	}

	constexpr Uint128t& operator|=(const Uint128t& other) noexcept {
		m_lower |= other.m_lower;
		m_upper |= other.m_upper;

		return *this;
	}

	constexpr Uint128t& operator^=(const Uint128t& other) noexcept {
		m_lower ^= other.m_lower;
		m_upper ^= other.m_upper;

		return *this;
	}
private:
	static constexpr std::uint64_t pow(std::uint64_t n, int m) noexcept {
		for (int i {0}; i < m; i++) {
			n *= n;
		}

		return n;
	}

	std::uint64_t m_lower {0};
	std::uint64_t m_upper {0};
};
