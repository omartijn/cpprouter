#pragma once

#include <type_traits>
#include <charconv>
#include <string_view>
#include <string>
#include "impl/variables.h"
#include "fields.h"


namespace router {

    /**
     *  Templated class describing bindings between
     *  slug data and struct members.
     *
     *  The struct is templated on the struct we need
     *  to bind to, as well as all the members that
     *  need binding.
     */
    template <typename data_type, auto... members>
    struct dto
    {
        /**
         *  Bind a slug to a field.
         *
         *  Fields are bound to slugs in order of appearance. The first field
         *  bound will be filled with the data from the first slug, the second
         *  field with that of the second slug, etc...
         *
         *  @tparam field   The field to bind
         */
        template <auto field>
        using bind = dto<data_type, members..., field>;

        /**
         *  Get the number of bound fields
         *
         *  @return The number of fields bound to the structure
         */
        constexpr static std::size_t size() noexcept
        {
            return sizeof...(members);
        }

        /**
         *  Parse all slug data into the given
         *  data transfer object.
         *
         *  @param  slugs   The slug data to parse
         *  @param  output  The object to fill
         */
        static void to_dto(const std::vector<std::string_view>& slugs, data_type& output)
        {
            // ensure the number of slugs is correct
            if (slugs.size() != size()) {
                // we cannot parse the data
                throw std::logic_error{ "Cannot convert slugs to dto: slug count mismatch" };
            }

            // iterator to use in the fold expression
            auto iter = begin(slugs);

            // fold the fields into the output object
            (process_field(*iter++, output.*members), ...);
        }
    };

    /**
     *  Type trait for a type not implementing
     *  the dto-specific requirements.
     */
    template <typename T, typename = void>
    struct is_dto_type : std::false_type {};

    /**
     *  Type trait for a valid dto-compatible type
     */
    template <typename T>
    struct is_dto_type<T, std::void_t<
        /**
         *  It must have a dto specialization on the type to be considered valid,
         *  and it must be able process a vector of slugs onto the given data type
         */
        std::enable_if_t<std::is_same_v<
            void,
            decltype(T::dto::to_dto(
                std::declval<const std::vector<std::string_view>&>(),
                std::declval<T&>()
            ))
        >>
    >> : std::true_type {};

    /**
     *  Value alias for dto type deduction
     */
    template <typename T>
    constexpr bool is_dto_type_v = is_dto_type<T>::value;


    /**
     *  Type trait for an std::tuple of processable fields
     */
    template <typename>
    struct is_dto_tuple : std::false_type {};

    /**
     *  Type trait for a valid tuple match
     */
    template <typename... T>
    struct is_dto_tuple<std::tuple<T...>> :
        std::integral_constant<bool, (... && is_processable_field<T>::value)> {};

    /**
     *  Value alias for dto tuple deduction
     */
    template <typename T>
    constexpr bool is_dto_tuple_v = is_dto_tuple<T>::value;

    /**
     *  Read a vector of slugs and parse them
     *  into the given dto type
     */
    template <typename data_type>
    std::enable_if_t<is_dto_type_v<data_type>>
    to_dto(const std::vector<std::string_view>& slugs, data_type& output)
    {
        // invoke the conversion routine on the types dto alias
        data_type::dto::to_dto(slugs, output);
    }

    /**
     *  Read a vector of slugs and parse them
     *  into the given tuple
     */
    template <typename... types>
    std::enable_if_t<is_dto_tuple_v<std::tuple<types...>>>
    to_dto(const std::vector<std::string_view>& slugs, std::tuple<types...>& output)
    {
        // create integer sequence for retrieving types from the tuple
        impl::to_dto(slugs, output, std::make_index_sequence<sizeof...(types)>());
    }

}
