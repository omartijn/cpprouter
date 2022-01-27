#pragma once

#include "function_traits.h"
#include "wrap_callback.h"
#include <functional>
#include <string_view>
#include <vector>


namespace router {

    /**
     *  In-place option used for the constructor
     */
    template <auto>
    struct in_place_value {};

    /**
     *  Undefined templated class for specializing
     *  into a function-like template class
     */
    template <class>
    class path_callback;

    /**
     *  A callback, wrapped to be used with a path
     */
    template <class return_type, class... arguments>
    class path_callback<return_type(arguments...)>
    {
        public:
            /**
             *  Default constructor
             */
            path_callback() = default;

            /**
             *  Construct with a free function
             *
             *  @tparam callback    The callback to install
             */
            template <auto callback, typename = std::enable_if_t<!std::is_member_function_pointer_v<decltype(callback)>>>
            path_callback(in_place_value<callback>) noexcept :
                _callback{ &wrap_callback<callback> }
            {}

            /**
             *  Construct with a member function
             *
             *  @tparam callback    The callback to install
             *  @param  instance    The instance to invoke on
             */
            template <auto callback, typename = std::enable_if_t<std::is_member_function_pointer_v<decltype(callback)>>>
            path_callback(in_place_value<callback>, typename function_traits<decltype(callback)>::member_type* instance) noexcept :
                _callback{ &wrap_callback<callback> },
                _instance{ instance }
            {}

            /**
             *  Set a free function to be invoked
             *
             *  @tparam callback    The callback to install
             */
            template <auto callback>
            std::enable_if_t<!std::is_member_function_pointer_v<decltype(callback)>>
            set() noexcept
            {
                _callback = &wrap_callback<callback>;
            }

            /**
             *  Set a member function to be invoked
             *
             *  @tparam callback    The callback to install
             *  @param  instance    The instance to invoke on
             */
            template <auto callback>
            std::enable_if_t<std::is_member_function_pointer_v<decltype(callback)>>
            set(typename function_traits<decltype(callback)>::member_type* instance) noexcept
            {
                _callback = &wrap_callback<callback>;
                _instance = instance;
            }

            /**
             *  Check if we have a valid callback installed
             *
             *  @return Whether a callback was set
             */
            bool valid() const noexcept { return _callback != nullptr; }
            operator bool() const noexcept { return valid(); }

            /**
             *  Invoke the installed function
             *
             *  @param  slugs       The slugs parsed from the path
             *  @param  parameters  The parameters to the callback
             *  @throws std::bad_function_call  In case no member function is installed
             */
            return_type operator()(const std::vector<std::string_view>& slugs, arguments&&... parameters) const
            {
                // check whether we have a valid callback
                if (_callback == nullptr) {
                    throw std::bad_function_call{};
                }

                // invoke the callback
                return _callback(slugs, _instance, std::forward<arguments>(parameters)...);
            }
        private:
            /**
             *  Alias for a wrapped callback
             */
            using wrapped_callback = return_type(*)(const std::vector<std::string_view>& slugs, void* instance, arguments&&... parameters);

            wrapped_callback    _callback{};    // the callback to invoke
            void*               _instance{};    // the instance to invoke on (empty for non-member functions)
    };

}
