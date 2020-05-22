#pragma once

#include <string_view>
#include <type_traits>
#include <charconv>


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
     *  Type trait to check whether a field is processable
     *
     *  Default for when no valid match is made
     */
    template <typename T, typename = void>
    struct is_processable_field : std::false_type {};

    /**
     *  Match for a valid processable field
     */
    template <typename T>
    struct is_processable_field<T, std::void_t<
        std::enable_if_t<
            std::is_same_v<
                void,
                decltype(process_field(std::string_view{}, std::declval<T&>()))
            >
        >
    >> : std::true_type {};

}
