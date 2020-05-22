#include "catch2.hpp"
#include <router/tuple_slice.h>


TEST_CASE("We should be able to deduce a slice of a tuple")
{
    // check that slicing 0 elements works
    static_assert(
        std::is_same_v<
            typename router::tuple_slice<0, std::size_t>::type,
            std::tuple<std::size_t>
        >, "Slicing 0 elements from tuple breaks"
    );

    // now check whether slicing off 2 out of 4 elements
    static_assert(
        std::is_same_v<
            typename router::tuple_slice<2, std::size_t, uint64_t, float, double>::type,
            std::tuple<float, double>
        >, "Slicing 2 out of 4 elements breaks"
    );

    // check that empty parameter list works
    static_assert(
        std::is_same_v<
            typename router::tuple_slice<0>::type,
            std::tuple<>
        >, "Slicing an empty tuple breaks"
    );

    // check that consuming all types works
    static_assert(
        std::is_same_v<
            typename router::tuple_slice<1, float>::type,
            std::tuple<>
        >, "Slicing off all elements from a tuple breaks"
    );
}
