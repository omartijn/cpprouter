#pragma once

#include <algorithm>
#include <vector>
#include "path.h"


namespace router {

    /**
     *  A class mapping paths to something else
     */
    template <typename V>
    class path_map
    {
        public:
            using value_type = V;

            /**
             *  Add a value to the map
             *
             *  @param  endpoint    The endpoint to add
             *  @param  parameters  The parameters to create the value
             */
            template <typename... arguments>
            value_type& add(std::string_view endpoint, arguments&&... parameters)
            {
                // create the path to route
                path    path    { endpoint  };

                // does the endpoint contain a fixed prefix?
                if (path.prefix().empty()) {
                    // there is no prefix to sort on so
                    // we add this to the unsorted list
                     return _unsorted_paths.emplace_back(
                        std::piecewise_construct,
                        std::forward_as_tuple(std::move(path)),
                        std::forward_as_tuple(std::forward<arguments>(parameters)...)
                    ).second;
                } else {
                    // find the element to insert before
                    auto iter = std::lower_bound(begin(_prefixed_paths), end(_prefixed_paths), path, [](const auto& a, const auto& b) {
                        // endpoints are sorted by prefix
                        return std::get<0>(a).prefix() < b.prefix();
                    });

                    // add it to the sorted list before the found element
                    return _prefixed_paths.emplace(iter,
                        std::piecewise_construct,
                        std::forward_as_tuple(std::move(path)),
                        std::forward_as_tuple(std::forward<arguments>(parameters)...)
                    )->second;
                }
            }

            /**
             *  Find an entry in the map
             *
             *  @param  slugs       The slugs to fill if found
             *  @param  endpoint    The endpoint to lookup
             *  @return The found value, or a nullptr
             */
            const value_type* find(std::vector<std::string_view>& slugs, std::string_view endpoint) const noexcept
            {
                // find the first possibly matching entry by prefix
                auto iter = std::lower_bound(begin(_prefixed_paths), end(_prefixed_paths), endpoint, [](const auto& a, const auto& b) {
                    // check whether the prefix comes before the given endpoint
                    return std::get<0>(a).prefix() < b.substr(0, std::get<0>(a).prefix().size());
                });

                // try all valid matches
                while (iter != end(_prefixed_paths)) {
                    // retrieve the path, callback and instance
                    const auto& [path, value] = *iter;

                    // if the prefix no longer matches we have exhausted all options
                    if (!path.match_prefix(endpoint)) {
                        // stop now to avoid expensive regex calls
                        break;
                    }

                    // try to match the path to the given endpoint
                    if (path.match(endpoint, slugs)) {
                        // we matched the endpoint, return the handler
                        return &value;
                    }

                    // move on to the next entry
                    ++iter;
                }

                // none of the prefixed paths matched, so try the
                // unsorted paths without a known prefix
                for (const auto& [path, value] : _unsorted_paths) {
                    // try to match the path to the given endpoint
                    if (path.match(endpoint, slugs)) {
                        // we matched the endpoint, return the handler
                        return &value;
                    }
                }

                // none of the paths matched
                return nullptr;
            }
        private:
            /**
             *  The entry type we store inside the map, we store both the
             *  path and the given value type together in a flat structure
             */
            using entry = std::pair<path, value_type>;

            std::vector<entry>  _prefixed_paths;    // a sorted list of paths with prefixes
            std::vector<entry>  _unsorted_paths;    // paths without a prefix, not sorted
    };

}
