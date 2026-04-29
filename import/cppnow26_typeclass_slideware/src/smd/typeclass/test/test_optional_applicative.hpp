#ifndef INCLUDE_SMD_TYPECLASS_TEST_TEST_OPTIONAL_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_TEST_TEST_OPTIONAL_APPLICATIVE_HPP

#include <smd/typeclass/applicative.hpp>

#include <beman/optional/optional.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd::typeclass {

template <class VALUE_TYPE>
struct Applicative<beman::optional::optional<VALUE_TYPE> > {
    template <class VALUE>
    constexpr auto pure(VALUE&& value) const
        -> beman::optional::optional<std::remove_cvref_t<VALUE> >;

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    constexpr auto apply(FUNCTION_IN_CONTEXT&& function,
                         ARGUMENT_IN_CONTEXT&& argument) const;

    template <class FUNCTION, class... ARGUMENTS>
    constexpr auto invoke(FUNCTION&& function, ARGUMENTS&&... arguments) const;
};

template <class VALUE_TYPE>
template <class VALUE>
constexpr auto Applicative<beman::optional::optional<VALUE_TYPE> >::pure(
    VALUE&& value) const -> beman::optional::optional<std::remove_cvref_t<VALUE> >
{
    return beman::optional::optional<std::remove_cvref_t<VALUE> >{
        std::forward<VALUE>(value)};
}

template <class VALUE_TYPE>
template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
constexpr auto Applicative<beman::optional::optional<VALUE_TYPE> >::apply(
    FUNCTION_IN_CONTEXT&& function,
    ARGUMENT_IN_CONTEXT&& argument) const
{
    using Result = std::invoke_result_t<decltype(*function), decltype(*argument)>;

    if (!function || !argument) {
        return beman::optional::optional<std::remove_cvref_t<Result> >{};
    }

    return beman::optional::optional<std::remove_cvref_t<Result> >{
        std::invoke(*std::forward<FUNCTION_IN_CONTEXT>(function),
                    *std::forward<ARGUMENT_IN_CONTEXT>(argument))};
}

template <class VALUE_TYPE>
template <class FUNCTION, class... ARGUMENTS>
constexpr auto Applicative<beman::optional::optional<VALUE_TYPE> >::invoke(
    FUNCTION&& function,
    ARGUMENTS&&... arguments) const
{
    using Result = std::invoke_result_t<FUNCTION, decltype(*arguments)...>;

    if (!(arguments && ...)) {
        return beman::optional::optional<std::remove_cvref_t<Result> >{};
    }

    return beman::optional::optional<std::remove_cvref_t<Result> >{
        std::invoke(std::forward<FUNCTION>(function),
                    *std::forward<ARGUMENTS>(arguments)...)};
}

}  // close namespace smd::typeclass

#endif  // INCLUDE_SMD_TYPECLASS_TEST_TEST_OPTIONAL_APPLICATIVE_HPP
