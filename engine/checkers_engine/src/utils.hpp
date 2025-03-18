#pragma once

// https://en.cppreference.com/w/cpp/utility/unreachable

// We don't have C++23 yet :|

namespace utils {
    [[noreturn]] inline void unreachable() noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
        __assume(false);
#else
        __builtin_unreachable();
#endif
    }
}
