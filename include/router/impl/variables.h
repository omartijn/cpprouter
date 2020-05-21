#pragma once

#include <utility>
#include "../fields.h"


namespace router::impl {

    /**
     *  Read a vector of slugs and parse them
     *  into the given tuple
     */
    template <typename... types, std::size_t... I>
    void to_dto(const std::vector<std::string_view>& slugs, std::tuple<types...>& output, std::index_sequence<I...>)
    {
        // ensure the number of slugs is correct
        if (slugs.size() != sizeof...(types)) {
            // we cannot parse the data
            throw std::logic_error{ "Cannot convert slugs to dto: slug count mismatch" };
        }

        // iterator to use in the fold expression
        auto iter = begin(slugs);

        // fold the fields into the output object
        (process_field(*iter++, std::get<I>(output)), ...);
    }

}
