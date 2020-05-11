#pragma once

#include <type_traits>
#include <charconv>
#include <string_view>
#include <string>


namespace router {

    /**
     *  Process a simple string field
     *
     *  @param  input   The slug data
     *  @param  output  The field to set
     */
    inline void process_field(std::string_view input, std::string& output)
    {
        // assign the input data to the output string
        output.assign(input);
    }

    /**
     *  Process a field containing numerical data
     *
     *  @param  input   The slug data
     *  @param  output  The field to set
     */
    template <typename integral>
    std::enable_if_t<std::is_arithmetic_v<integral>>
    process_field(std::string_view input, integral& output)
    {
        // first parse the input data
        auto result = std::from_chars(input.data(), std::next(input.data(), input.size()), output, 10);

        // check whether we successfully parsed the data and whether all of it was parsed
        if (result.ec != std::errc{}) {
            // some of the data was invalid and could not be converted
            throw std::range_error{ std::make_error_code(result.ec).message() };
        } else if (result.ptr != std::next(input.data(), input.size())) {
            // part of the data contained numeric input, but not all of it
            throw std::range_error{ "Input data contains non-numerical data" };
        }
    }

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

}
