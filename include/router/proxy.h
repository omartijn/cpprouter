#pragma once

#include "variadic_lookup.h"
#include "function_traits.h"
#include "wrap_callback.h"
#include "path_callback.h"
#include <string_view>
#include <vector>
#include <array>


namespace router {

    /**
     *  Undefined templated class for specializing
     *  into a function-like template class
     */
    template <class, auto, auto...>
    class proxy;

	/**
	 *  A proxy for forwarding a single route to a set of
	 *  different callbacks, depending on an extra method.
	 */
    template <class return_type, class... arguments, auto first, decltype(first)... rest>
	class proxy<return_type(arguments...), first, rest...>
	{
        public:
            /**
             *  Alias for the callback type proxied to
             */
            using callback_type = path_callback<return_type(arguments...)>;

            /**
             *  Retrieve an installed handler
             *
             *  @param  method  The method to retrieve the handler for
             *  @return The handler, or a nullptr if not set
             */
            const callback_type& get(decltype(first) method) const noexcept
            {
                // the index to retrieve
                auto index = method_index(method);

                // retrieve the callback
                return _callbacks[index];
            }

            /**
             *  Set up a handler
             *
             *  @tparam method      The method to register under
             *  @tparam callback    The callback to register
             */
            template <decltype(first) method, auto callback>
            std::enable_if_t<!std::is_member_function_pointer_v<decltype(callback)>, proxy&>
            set() noexcept
            {
                // the index to store under
                constexpr auto index = method_index<method>();

                // wrap and store the callback
                _callbacks[index].template set<callback>();

                // allow chaining
                return *this;
            }

            /**
             *  Set up a handler
             *
             *  @tparam method      The method to register under
             *  @tparam callback    The callback to register
             *  @param  instance    The instance to invoke the callback on
             */
            template <decltype(first) method, auto callback>
            std::enable_if_t<std::is_member_function_pointer_v<decltype(callback)>, proxy&>
            set(typename function_traits<decltype(callback)>::member_type* instance) noexcept
            {
                // the index to store under
                constexpr auto index = method_index<method>();

                // wrap and store the callback
                _callbacks[index].template set<callback>(instance);

                // allow chaining
                return *this;
            }
        private:
            /**
             *  Retrieve the index for the method
             *  in the list of methods
             *
             *  @param  method The method to lookup
             */
            static std::size_t method_index(decltype(first) method)
            {
                return variadic_lookup<0, first, rest...>(method);
            }

            /**
             *  Retrieve the index for the method
             *  in the list of methods
             *
             *  @tparam method  The property to lookup
             */
            template <decltype(first) method>
            constexpr static std::size_t method_index()
            {
                return variadic_lookup<method, first, rest...>();
            }

            /**
             *  The number of available options, note that we
             *  add one to the count for the first argument
             */
            constexpr static std::size_t count = 1 + sizeof...(rest);

            /**
             *  All the registered callbacks
             */
            std::array<callback_type, count> _callbacks{};
	};

}
