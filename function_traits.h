#pragma once

#include <tuple>


namespace router {

    /**
     *  Unspecialized function traits
     */
    template <typename unused>
    struct function_traits;

    /**
     *  Specialize for a function type
     */
    template <typename R, typename... Args>
    struct function_traits<R(*)(Args...)>
    {
        using return_type   = R;
        using argument_type = std::tuple<Args...>;
    };

    /**
     *  Specialize for a member function type
     */
    template <typename R, typename... Args, typename X>
    struct function_traits<R(X::*)(Args...)>
    {
        using member_type   = X;
        using return_type   = R;
        using argument_type = std::tuple<Args...>;
    };

}
