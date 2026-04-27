#ifndef INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP

#include <smd/typeclass/typeclass_base.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd::typeclass {

template <class APPLICATIVE>
struct Applicative;

template <class APPLICATIVE>
inline constexpr Applicative<RemoveCvRef<APPLICATIVE> > applicative_v =
    Applicative<RemoveCvRef<APPLICATIVE> >{};

}  // close namespace smd::typeclass

namespace smd {

template <class APPLICATIVE, class VALUE>
constexpr decltype(auto) pure(VALUE&& value);

template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
constexpr decltype(auto) apply(FUNCTION_IN_CONTEXT&& function,
                               ARGUMENT_IN_CONTEXT&& argument);

template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
constexpr decltype(auto) invoke(FUNCTION&& function,
                                FIRST_ARGUMENT&& firstArgument,
                                REST_ARGUMENTS&&... restArguments);

}  // close namespace smd

namespace smd {

template <class APPLICATIVE, class VALUE>
constexpr decltype(auto) pure(VALUE&& value)
{
    return typeclass::applicative_v<APPLICATIVE>.pure(
        std::forward<VALUE>(value));
}

template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
constexpr decltype(auto) apply(FUNCTION_IN_CONTEXT&& function,
                               ARGUMENT_IN_CONTEXT&& argument)
{
    return typeclass::applicative_v<FUNCTION_IN_CONTEXT>.apply(
        std::forward<FUNCTION_IN_CONTEXT>(function),
        std::forward<ARGUMENT_IN_CONTEXT>(argument));
}

template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
constexpr decltype(auto) invoke(FUNCTION&& function,
                                FIRST_ARGUMENT&& firstArgument,
                                REST_ARGUMENTS&&... restArguments)
{
    return typeclass::applicative_v<FIRST_ARGUMENT>.invoke(
        std::forward<FUNCTION>(function),
        std::forward<FIRST_ARGUMENT>(firstArgument),
        std::forward<REST_ARGUMENTS>(restArguments)...);
}

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
