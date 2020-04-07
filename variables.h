#pragma once

#include <string_view>
#include <string>


namespace router {

    /**
     *  Process a simple string field
     *
     *  @param  input   The slug data
     *  @param  output  The field to set
     */
    void process_field(std::string_view input, std::string& output)
    {
        // assign the input data to the output string
        output.assign(input);
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
        static void to_dto(const std::array<std::string_view, size()>& slugs, data_type& output)
        {
            // iterator to use in the fold expression
            auto iter = begin(slugs);

            // fold the fields into the output object
            (process_field(*iter++, output.*members), ...);
        }
    };

}
