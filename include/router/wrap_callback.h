#pragma once

#include "function_traits.h"
#include "variables.h"
#include <string_view>
#include <vector>


namespace router {

    /**
     *  Wrap a callback to create a uniform handler
     *
     *  @tparam callback    The callback to wrap
     *  @param  slugs       The slug data from the target
     *  @param  instance    The instance to invoke the callback on
     *  @param  parameters  Additional arguments to pass to the callback
     */
    template <auto callback, typename return_type, typename... arguments>
    return_type wrap_callback(const std::vector<std::string_view>& slugs, void* instance, arguments&&... parameters)
    {
        // the number of arguments our callback function takes
        // ignoring the extra variable parameter it may take
        constexpr std::size_t arity = sizeof...(arguments);

        // traits for the callback function, and the data type used for the variables
        using traits = function_traits<decltype(callback)>;

        // does the callback require slug parsing?
        if constexpr (traits::arity == arity) {
            // is it a member function?
            if constexpr (!traits::is_member_function) {
                // it is not, we can invoke the callback directly
                return callback(std::forward<arguments>(parameters)...);
            } else {
                // invoke the function on the given instance
                return (static_cast<typename traits::member_type*>(instance)->*callback)(std::forward<arguments>(parameters)...);
            }
        } else if constexpr (traits::arity == arity + 1 && (is_dto_type_v<std::remove_reference_t<typename traits::template argument_type<arity>>> || is_dto_tuple_v<std::remove_reference_t<typename traits::template argument_type<arity>>>)) {
            // determine the type used for the slug data, this comes as the optional
            // last parameter the function may take, elements are zero-based
            using variable_type = std::remove_reference_t<typename traits::template argument_type<arity>>;

            // create the variables to be filled and try to match the target
            variable_type variables{};

            // parse the variables
            to_dto(slugs, variables);

            // is it a member function?
            if constexpr (!traits::is_member_function) {
                // invoke the wrapped callback and return the result
                return callback(std::forward<arguments>(parameters)..., std::move(variables));
            } else {
                // invoke the function on the given instance
                return (static_cast<typename traits::member_type*>(instance)->*callback)(std::forward<arguments>(parameters)..., std::move(variables));
            }
        } else {
            // the variable type is a tuple of the slug arguments
            // which is unpacked later using std::apply
            using variable_type = typename traits::template arguments_slice<arity>;

            // create the variables to be filled and try to match the target
            variable_type variables{};

            // parse the variables
            to_dto(slugs, variables);

            // is it a member function?
            if constexpr (!traits::is_member_function) {
                // invoke the wrapped callback and return the result
                return std::apply(callback, std::tuple_cat(
                    std::forward_as_tuple(std::forward<arguments>(parameters)...),
                    std::move(variables)
                ));
            } else {
                // invoke the function on the given instance
                return std::apply(callback, std::tuple_cat(
                    std::forward_as_tuple(static_cast<typename traits::member_type*>(instance)),
                    std::forward_as_tuple(std::forward<arguments>(parameters)...),
                    std::move(variables)
                ));
            }
        }
    }

}
