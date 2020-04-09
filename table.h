#pragma once

#include <algorithm>
#include <vector>
#include "path.h"
#include "function_traits.h"


namespace router {

    /**
     *  Undefined templated class for specializing
     *  into a function-like template class
     */
    template <class>
    class table;

    /**
     *  A routing table, containing paths to route and mapping
     *  them to their associated callbacks.
     */
    template <class return_type, class... arguments>
    class table<return_type(arguments...)>
    {
        public:
            /**
             *  Add an endpoint to the routing table
             *
             *  @tparam callback    The callback to route to
             *  @param  endpoint    The path to add
             */
            template <auto callback>
            void add(std::string_view endpoint)
            {
                // create the path to route
                path    path    { endpoint  };

                // does the endpoint contain a fixed prefix?
                if (path.prefix().empty()) {
                    // there is no prefix to sort on so
                    // we add this to the unsorted list
                    _unsorted_paths.emplace_back(std::move(path), &table::wrap_callback<callback>);
                } else {
                    // find the element to insert before
                    auto iter = std::lower_bound(begin(_prefixed_paths), end(_prefixed_paths), path, [](const auto& a, const auto& b) {
                        // endpoints are sorted by prefix
                        return a.first.prefix() < b.prefix();
                    });

                    // add it to the sorted list before the found element
                    _prefixed_paths.emplace(iter, std::move(path), &table::wrap_callback<callback>);
                }
            }

            /**
             *  Route a request to one of the callbacks
             *
             *  @param  endpoint    The endpoint to route
             *  @param  arguments   The arguments to give to the callback
             *  @return The result of the callback
             *  @throws std::out_of_range
             */
            return_type route(std::string_view endpoint, arguments... parameters)
            {
                // local vector for capturing slugs, this is static
                // so the storage gets re-used, saving on (de)allocations
                thread_local std::vector<std::string_view> slugs;

                // find the first possibly matching entry by prefix
                auto iter = std::lower_bound(begin(_prefixed_paths), end(_prefixed_paths), endpoint, [](const auto& a, const auto& b) {
                    // check whether the prefix comes before the given endpoint
                    return a.first.prefix() < b.substr(0, a.first.prefix().size());
                });

                // try all valid matches
                while (iter != end(_prefixed_paths) && iter->first.match_prefix(endpoint)) {
                    // try to match the path to the given endpoint
                    if (iter->first.match(endpoint, slugs)) {
                        // we matched the endpoint, invoke the callback
                        return iter->second(slugs, std::forward<arguments>(parameters)...);
                    }

                    // move on to the next entry
                    ++iter;
                }

                // none of the prefixed paths matched, so try the
                // unsorted paths without a known prefix
                for (const auto& [path, callback] : _unsorted_paths) {
                    // try to match the path to the given endpoint
                    if (path.match(endpoint, slugs)) {
                        // we matched the endpoint, invoke the callback
                        return callback(slugs, std::forward<arguments>(parameters)...);
                    }
                }

                // none of the paths matched
                throw std::out_of_range{ "Route not matched" };
            }
        private:
            /**
             *  Alias for a wrapped callback
             */
            using wrapped_callback = return_type(*)(const std::vector<std::string_view>& slugs, arguments... parameters);

            /**
             *  Alias for a routed path and the callback
             */
            using entry = std::pair<path, wrapped_callback>;

            /**
             *  Wrap a callback to create a uniform handler
             *
             *  @tparam callback    The callback to wrap
             *  @param  slugs       The slug data from the target
             *  @param  arguments   Additional arguments to pass to the callback
             */
            template <auto callback>
            static return_type wrap_callback(const std::vector<std::string_view>& slugs, arguments... parameters)
            {
                // the number of arguments our callback function takes
                // ignoring the extra variable parameter it may take
                constexpr std::size_t arity = sizeof...(arguments);

                // traits for the callback function, and the data type used for the variables
                using traits        = function_traits<decltype(callback)>;
                using variable_type = std::remove_reference_t<std::tuple_element_t<arity, typename traits::argument_type>>;

                // create the variables to be filled and try to match the target
                variable_type   variables{};

                // parse the variables
                variable_type::dto::to_dto(slugs, variables);

                // invoke the wrapped callback and return the result
                return callback(std::forward<arguments>(parameters)..., std::move(variables));
            }

            std::vector<entry>  _prefixed_paths;    // a sorted list of paths with prefixes
            std::vector<entry>  _unsorted_paths;    // paths without a prefix, not sorted
    };

}
