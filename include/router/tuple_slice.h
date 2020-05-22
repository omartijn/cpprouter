#pragma once

#include <tuple>


namespace router {

    /**
     *  Unimplemented base template
     *  for tuple slice. It is implemented
     *  in various specialisations below.
     */
    template <std::size_t offset, typename... elements>
    struct tuple_slice;

    /**
     *  Specialisation when at least one
     *  element has to still be removed
     */
    template <std::size_t offset, typename element, typename... remaining>
    struct tuple_slice<offset, element, remaining...>
    {
        using type = typename tuple_slice<offset - 1, remaining...>::type;
    };

    /**
     *  Specialisation for when there are no
     *  types to be removed
     */
    template <>
    struct tuple_slice<0>
    {
        using type = std::tuple<>;
    };

    template <typename element>
    struct tuple_slice<0, element>
    {
        using type = std::tuple<element>;
    };

    template <typename element, typename... remaining>
    struct tuple_slice<0, element, remaining...>
    {
        using type = std::tuple<element, remaining...>;
    };

    /**
     *  Type alias for tuple slice
     */
    template <std::size_t offset, typename... elements>
    using tuple_slice_t = typename tuple_slice<offset, elements...>::type;

}
