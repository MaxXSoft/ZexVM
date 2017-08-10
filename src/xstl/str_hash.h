// http://www.klayge.org/?p=2201

#ifndef XSTL_STR_HASH_H_
#define XSTL_STR_HASH_H_

#include <cstddef>

namespace xstl {

constexpr size_t _Hash(const char (&str)[1]) {
    return *str + 0x9e3779b9;
}

template <size_t N>
constexpr size_t _Hash(const char (&str)[N]) {
    using TruncatedStr = const char (&)[N - 1];
    #define seed _Hash((TruncatedStr)str)
    return seed ^ (*(str + N - 1) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    #undef seed
}

template <size_t N>
constexpr size_t StringHash(const char (&str)[N]) {
    using TruncatedStr = const char (&)[N - 1];
    return _Hash<N - 1>((TruncatedStr)str);
}

// different from StringHash
inline size_t StringHashRT(const char *str) {
    size_t hash = 0, seed = 131;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return hash;
}

} // namespace xstl

#endif // XSTL_STR_HASH_H_
