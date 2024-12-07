#pragma once

#include <utility>
#include <type_traits>
#include <iterator>
#include <cstddef>
#include <cassert>

namespace array {
    template<typename T, int Size>
    class Array {
    public:
        static_assert(
            std::is_nothrow_constructible_v<T> &&
            std::is_nothrow_destructible_v<T> &&
            std::is_nothrow_copy_assignable_v<T> &&
            std::is_nothrow_move_assignable_v<T>
        );

        void push_back(const T& item) noexcept {
            assert(m_size < Size);
            m_data[m_size++] = item;
        }

        void push_back(T&& item) noexcept {
            assert(m_size < Size);
            m_data[m_size++] = std::move(item);
        }

        template<typename... Args>
        void emplace_back(Args&&... args) noexcept {
            assert(m_size < Size);
            auto address {&m_data[m_size++]};
            new (address) T(std::forward<Args>(args)...);
        }

        void pop_back() noexcept {
            assert(m_size > 0);
            m_size--;
        }

        const T& operator[](int index) const noexcept {
            assert(index < m_size);
            return m_data[index];
        }

        T& operator[](int index) noexcept {
            assert(index < m_size);
            return m_data[index];
        }

        const T& back() const noexcept {
            return m_data[m_size - 1];
        }

        T& back() noexcept {
            return m_data[m_size - 1];
        }

        int size() const noexcept {
            return m_size;
        }

        bool empty() const noexcept {
            return m_size == 0;
        }

        void clear() noexcept {
            m_size = 0;
        }

        class const_iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = const T*;
            using reference = const T&;
            using iterator_category = std::input_iterator_tag;

            explicit const_iterator(int index, pointer data) noexcept
                : m_index(index), m_data(data) {}

            const_iterator& operator++() noexcept {
                m_index++;
                return *this;
            }

            const_iterator operator++(int) noexcept {
                const_iterator self {*this};
                ++(*this);
                return self;
            }

            bool operator==(const_iterator other) const noexcept {
                return m_index == other.m_index;
            }

            bool operator!=(const_iterator other) const noexcept {
                return !(*this == other);
            }

            reference operator*() const noexcept {
                return m_data[m_index];
            }
        private:
            int m_index;
            pointer m_data;
        };

        class iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::input_iterator_tag;

            explicit iterator(int index, pointer data) noexcept
                : m_index(index), m_data(data) {}

            iterator& operator++() noexcept {
                m_index++;
                return *this;
            }

            iterator operator++(int) noexcept {
                iterator self {*this};
                ++(*this);
                return self;
            }

            bool operator==(iterator other) const noexcept {
                return m_index == other.m_index;
            }

            bool operator!=(iterator other) const noexcept {
                return !(*this == other);
            }

            reference operator*() noexcept {
                return m_data[m_index];
            }
        private:
            int m_index;
            pointer m_data;
        };

        const_iterator begin() const noexcept {
            return const_iterator(0, m_data);
        }

        const_iterator end() const noexcept {
            return const_iterator(m_size, m_data);
        }

        iterator begin() noexcept {
            return iterator(0, m_data);
        }

        iterator end() noexcept {
            return iterator(m_size, m_data);
        }
    private:
        T m_data[Size];
        int m_size {};
    };
};
