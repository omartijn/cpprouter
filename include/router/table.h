#pragma once

#include <algorithm>
#include <vector>
#include <tuple>
#include "path.h"
#include "variables.h"
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
            std::enable_if_t<!std::is_member_function_pointer_v<decltype(callback)>>
            add(std::string_view endpoint)
            {
                // add the endpoint using the wrapped callback, since
                // the callback is not a member functio we need no instance
                add(&table::wrap_callback<callback>, endpoint, nullptr);
            }

            /**
             *  Add an endpoint to the routing table
             *
             *  @tparam callback    The callback to route to
             *  @param  endpoint    The path to add
             *  @param  instance    The instance to invoke the callback on
             */
            template <auto callback>
            std::enable_if_t<std::is_member_function_pointer_v<decltype(callback)>>
            add(std::string_view endpoint, typename function_traits<decltype(callback)>::member_type* instance)
            {
                // add the endpoint using the wrapped callback
                add(&table::wrap_callback<callback>, endpoint, instance);
            }

            /**
             *  Set a handler for endpoints that are not found
             *
             *  @tparam callback    The callback to route to
             */
            template <auto callback>
            std::enable_if_t<!std::is_member_function_pointer_v<decltype(callback)>>
            set_not_found()
            {
                // store the function pointer, there is no instance
                _not_found_handler.first    = &table::wrap_callback<callback>;
                _not_found_handler.second   = nullptr;
            }

            /**
             *  Set a handler for endpoints that are not found
             *
             *  @tparam callback    The callback to route to
             *  @param  instance    The instance to invoke the callback on
             */
            template <auto callback>
            std::enable_if_t<std::is_member_function_pointer_v<decltype(callback)>>
            set_not_found(typename function_traits<decltype(callback)>::member_type* instance)
            {
                // store the function pointer and instance
                _not_found_handler.first    = &table::wrap_callback<callback>;
                _not_found_handler.second   = instance;
            }

            /**
             *  Check whether a given endpoint can be routed
             *
             *  Note that this function ignores the not-found
             *  handler, since this would otherwise make this
             *  function return true unconditionally.
             *
             *  @param  endpoint    The endpoint to check
             *  @return Whether the endpoint can be routed
             */
            bool routable(std::string_view endpoint) const noexcept
            {
                // find the handler for the given endpoint
                auto [callback, _] = get_handler(endpoint);

                // check whether the callback is valid
                return callback != nullptr;
            }

            /**
             *  Check whether a fallback handler is installed
             *
             *  If a fallback handler is installed and no endpoint is
             *  available when routing, the fallback handler is used.
             *  This also means that routing will never fail due to a
             *  missing endpoint.
             *
             *  @return Whether a fallback handler is installed
             */
            bool has_not_found_handler() const noexcept
            {
                return _not_found_handler.first != nullptr;
            }

            /**
             *  Route a request to one of the callbacks
             *
             *  @param  endpoint    The endpoint to route
             *  @param  parameters  The arguments to give to the callback
             *  @return The result of the callback
             *  @throws std::out_of_range
             */
            return_type route(std::string_view endpoint, arguments... parameters) const
            {
                // find the handler for the given endpoint
                if (auto [callback, instance] = get_handler(endpoint); callback != nullptr) {
                    // invoke the callback
                    return callback(slugs(), instance, std::forward<arguments>(parameters)...);
                }

                // do we have a handler for endpoints that aren't registered
                if (_not_found_handler.first != nullptr) {
                    // extract the callback and instance
                    auto [callback, instance] = _not_found_handler;

                    // invoke the handler
                    return callback({}, instance, std::forward<arguments>(parameters)...);
                }

                // none of the paths matched
                throw std::out_of_range{ "Route not matched" };
            }
        private:
            /**
             *  Alias for a wrapped callback
             */
            using wrapped_callback = return_type(*)(const std::vector<std::string_view>& slugs, void* instance, arguments&&... parameters);

            /**
             *  Alias for a routed path and the callback
             */
            using entry = std::tuple<path, wrapped_callback, void*>;

            /**
             *  Fallback handler for endpoint not registered
             */
            using not_found_handler = std::pair<wrapped_callback, void*>;

            /**
             *  Wrap a callback to create a uniform handler
             *
             *  @tparam callback    The callback to wrap
             *  @param  slugs       The slug data from the target
             *  @param  instance    The instance to invoke the callback on
             *  @param  parameters  Additional arguments to pass to the callback
             */
            template <auto callback>
            static return_type wrap_callback(const std::vector<std::string_view>& slugs, void* instance, arguments&&... parameters)
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

            /**
             *  Add an endpoint to the routing table
             *
             *  @param  callback    The callback to route to
             *  @param  endpoint    The path to add
             *  @param  instance    The instance to invoke the callback on
             */
            void add(wrapped_callback callback, std::string_view endpoint, void* instance)
            {
                // create the path to route
                path    path    { endpoint  };

                // does the endpoint contain a fixed prefix?
                if (path.prefix().empty()) {
                    // there is no prefix to sort on so
                    // we add this to the unsorted list
                    _unsorted_paths.emplace_back(std::move(path), callback, instance);
                } else {
                    // find the element to insert before
                    auto iter = std::lower_bound(begin(_prefixed_paths), end(_prefixed_paths), path, [](const auto& a, const auto& b) {
                        // endpoints are sorted by prefix
                        return std::get<0>(a).prefix() < b.prefix();
                    });

                    // add it to the sorted list before the found element
                    _prefixed_paths.emplace(iter, std::move(path), callback, instance);
                }
            }

            /**
             *  Retrieve the handler for a specific endpoint
             *
             *  If a valid handler is found (i.e. the result contains a valid
             *  callback) then _slugs is filled with the found slug data.
             *
             *  @param  endpoint    The endpoint to find
             *  @return The found match, which may be invalid
             */
            std::pair<wrapped_callback, void*> get_handler(std::string_view endpoint) const noexcept
            {
                // find the first possibly matching entry by prefix
                auto iter = std::lower_bound(begin(_prefixed_paths), end(_prefixed_paths), endpoint, [](const auto& a, const auto& b) {
                    // check whether the prefix comes before the given endpoint
                    return std::get<0>(a).prefix() < b.substr(0, std::get<0>(a).prefix().size());
                });

                // try all valid matches
                while (iter != end(_prefixed_paths)) {
                    // retrieve the path, callback and instance
                    const auto& [path, callback, instance] = *iter;

                    // if the prefix no longer matches we have exhausted all options
                    if (!path.match_prefix(endpoint)) {
                        // stop now to avoid expensive regex calls
                        break;
                    }

                    // try to match the path to the given endpoint
                    if (path.match(endpoint, slugs())) {
                        // we matched the endpoint, return the handler
                        return { callback, instance };
                    }

                    // move on to the next entry
                    ++iter;
                }

                // none of the prefixed paths matched, so try the
                // unsorted paths without a known prefix
                for (const auto& [path, callback, instance] : _unsorted_paths) {
                    // try to match the path to the given endpoint
                    if (path.match(endpoint, slugs())) {
                        // we matched the endpoint, return the handler
                        return { callback, instance };
                    }
                }

                // none of the paths matched
                return {};
            }

            /**
             *  Retrieve the slug data
             *
             *  @note   This data is stored locally per thread to ensure
             *          that the table remains thread-safe, and can re-use
             *          allocated data between calls.
             *
             *  @return The slug data
             */
            static std::vector<std::string_view>& slugs() noexcept
            {
                // the slugs to cache
                thread_local std::vector<std::string_view> slugs;
                return slugs;
            }

            std::vector<entry>  _prefixed_paths;                            // a sorted list of paths with prefixes
            std::vector<entry>  _unsorted_paths;                            // paths without a prefix, not sorted
            not_found_handler   _not_found_handler  { nullptr, nullptr  };  // the optional handler for paths not found
    };

}
