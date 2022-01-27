#pragma once

#include <vector>
#include "path.h"
#include "proxy.h"
#include "path_map.h"
#include "path_callback.h"
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
                // the callback is not a member function we need no instance
                _paths.add(endpoint, in_place_value<callback>{});
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
                _paths.add(endpoint, in_place_value<callback>{}, instance);
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
                _not_found_handler.template set<callback>();
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
                _not_found_handler.template set<callback>(instance);
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
                auto callback= get_handler(endpoint);

                // check whether the callback is valid
                return callback.valid();
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
                return _not_found_handler.valid();
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
                if (auto callback= get_handler(endpoint); callback.valid()) {
                    // invoke the callback
                    return callback(slugs(), std::forward<arguments>(parameters)...);
                }

                // do we have a handler for endpoints that aren't registered
                if (_not_found_handler) {
                    // invoke the handler
                    return _not_found_handler({}, std::forward<arguments>(parameters)...);
                }

                // none of the paths matched
                throw std::out_of_range{ "Route not matched" };
            }
        private:
            /**
             *  Alias for the callback type used by the routing table
             */
            using callback_type = path_callback<return_type(arguments...)>;

            /**
             *  Retrieve the handler for a specific endpoint
             *
             *  If a valid handler is found (i.e. the result contains a valid
             *  callback) then _slugs is filled with the found slug data.
             *
             *  @param  endpoint    The endpoint to find
             *  @return The found match, which may be invalid
             */
            callback_type get_handler(std::string_view endpoint) const noexcept
            {
                if (auto* handler = _paths.find(slugs(), endpoint); handler != nullptr) {
                    return *handler;
                } else {
                    return {};
                }
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

            path_map<callback_type> _paths;             // all registered paths in the table
            callback_type           _not_found_handler; // the optional handler for paths not found
    };

    /**
     *  A routing table, containing paths to route and mapping
     *  them to a proxy for further processing
     */
    template <class return_type, class... arguments, auto first, decltype(first)... rest>
    class table<proxy<return_type(arguments...), first, rest...>>
    {
        public:
            /**
             *  The proxy we route to
             */
            using proxy_type = proxy<return_type(arguments...), first, rest...>;

            /**
             *  Add an endpoint to the routing table, the callbacks
             *  can be registered on the returned proxy
             *
             *  @param  endpoint    The path to add
             */
            proxy_type& add(std::string_view endpoint)
            {
                // add the endpoint to create a proxy
                return _paths.add(endpoint);
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
                _not_found_handler.template set<callback>();
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
                _not_found_handler.template set<callback>(instance);
            }

            /**
             *  Set a handler for endpoints without a method handler
             *
             *  @tparam callback    The callback to invoke
             */
            template <auto callback>
            std::enable_if_t<!std::is_member_function_pointer_v<decltype(callback)>>
            set_not_proxied()
            {
                // store the function pointer, there is no instance
                _not_proxied_handler.template set<callback>();
            }

            /**
             *  Set a handler for endpoints without a method handler
             *
             *  @tparam callback    The callback to invoke
             *  @param  instance    The instance to invoke the callback on
             */
            template <auto callback>
            std::enable_if_t<std::is_member_function_pointer_v<decltype(callback)>>
            set_not_proxied(typename function_traits<decltype(callback)>::member_type* instance)
            {
                // store the function pointer and instance
                _not_proxied_handler.template set<callback>(instance);
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
                return _paths.find(slugs(), endpoint) != nullptr;
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
                return _not_found_handler.valid();
            }

            /**
             *  Route a request to one of the callbacks
             *
             *  @param  endpoint    The endpoint to route
             *  @param  method      The method to proxy to
             *  @param  parameters  The arguments to give to the callback
             *  @return The result of the callback
             *  @throws std::out_of_range
             */
            return_type route(std::string_view endpoint, decltype(first) method,  arguments... parameters) const
            {
                // find the handler for the given endpoint
                if (auto proxy = _paths.find(slugs(), endpoint); proxy != nullptr) {
                    // do we have a handler for the method
                    if (proxy->get(method).valid()) {
                        // invoke the callback
                        return proxy->get(method)(slugs(), std::forward<arguments>(parameters)...);
                    } else if (_not_proxied_handler.valid()) {
                        // invoke the missing-method handler
                        return _not_proxied_handler({}, std::forward<arguments>(parameters)...);
                    }
                }

                // do we have a handler for endpoints that aren't registered
                if (_not_found_handler) {
                    // invoke the handler
                    return _not_found_handler({}, std::forward<arguments>(parameters)...);
                }

                // none of the paths matched
                throw std::out_of_range{ "Route not matched" };
            }
        private:
            /**
             *  Alias for the callback type used by the routing table
             */
            using callback_type = path_callback<return_type(arguments...)>;

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

            path_map<proxy_type>    _paths;                 // all registered paths in the table
            callback_type           _not_found_handler;     // the optional handler for paths not found
            callback_type           _not_proxied_handler;   // the optional handler for when a method is not proxied
    };

}
