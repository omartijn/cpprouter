#pragma once

#include <stdexcept>
#include <cstddef>


namespace router {

    template <std::size_t index = 0, auto straw, decltype(straw)... haystack>
    constexpr std::size_t variadic_lookup(decltype(straw) needle)
    {
        if (needle == straw) {
            return index;
        } else if constexpr(sizeof...(haystack) > 0) {
            return variadic_lookup<index + 1, haystack...>(needle);
        } else {
            throw std::invalid_argument{ "Lookup failed: needle not found in variadic list" };
        }
    }

    template <auto needle, decltype(needle) straw, decltype(needle)... haystack, std::size_t index = 0>
    constexpr std::size_t variadic_lookup()
    {
        if constexpr(needle == straw) {
            return index;
        } else if constexpr (sizeof...(haystack) > 0) {
            return variadic_lookup<needle, haystack..., index + 1>();
        } else {
            static_assert(sizeof...(haystack) > 0, "Needle not found in haystack");
        }
    }

}
