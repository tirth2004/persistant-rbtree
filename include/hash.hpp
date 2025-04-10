#ifndef HASH_HPP
#define HASH_HPP

#include <string>
#include <type_traits>

// 64-bit FNV-1a constants
constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
constexpr uint64_t FNV_PRIME = 1099511628211ULL;

struct FNV1aHasher {
    template<typename Key>
    uint64_t operator()(const Key& key) const {
        std::string str = to_string_flex(key);
        return fnv1a(str);
    }

private:
    uint64_t fnv1a(const std::string& str) const {
        uint64_t hash = FNV_OFFSET_BASIS;
        for (char c : str) {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    // Helpers to convert key to string
    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, std::string>
    to_string_flex(const T& val) const {
        return std::to_string(val);
    }

    std::string to_string_flex(const std::string& val) const {
        return val;
    }

    std::string to_string_flex(const char* val) const {
        return std::string(val);
    }

    std::string to_string_flex(char c) const {
        return std::string(1, c);
    }
};

#endif