#ifndef INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP

#include <smd/typeclass/typeclass_base.hpp>

#include <beman/optional/optional.hpp>

#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

struct applicative_tag {};

template <class CONTEXT, class VALUE>
auto pure(VALUE&& value)
{
  using ContextType = remove_cvref_t<CONTEXT>;
  return map<applicative_tag, ContextType>::pure(std::forward<VALUE>(value));
}

template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
auto apply(FUNCTION_IN_CONTEXT&& function, ARGUMENT_IN_CONTEXT&& argument)
{
  using FunctionContext = remove_cvref_t<FUNCTION_IN_CONTEXT>;
  return map<applicative_tag, FunctionContext>::apply(
    std::forward<FUNCTION_IN_CONTEXT>(function),
    std::forward<ARGUMENT_IN_CONTEXT>(argument));
}

template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
auto invoke(FUNCTION&& function,
      FIRST_ARGUMENT&& first_argument,
      REST_ARGUMENTS&&... rest_arguments)
{
  using ContextType = remove_cvref_t<FIRST_ARGUMENT>;
  return map<applicative_tag, ContextType>::invoke(
    std::forward<FUNCTION>(function),
    std::forward<FIRST_ARGUMENT>(first_argument),
    std::forward<REST_ARGUMENTS>(rest_arguments)...);
}

template <class VALUE_TYPE>
struct map<applicative_tag, std::optional<VALUE_TYPE> > {
  template <class VALUE>
  static auto pure(VALUE&& value) -> std::optional<remove_cvref_t<VALUE> >
  {
    return std::optional<remove_cvref_t<VALUE> >{std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  static auto apply(FUNCTION_IN_CONTEXT&& function,
            ARGUMENT_IN_CONTEXT&& argument)
  {
    using Result =
      std::invoke_result_t<decltype(*function), decltype(*argument)>;

    if (!function || !argument) {
      return std::optional<remove_cvref_t<Result> >{};
    }

    return std::optional<remove_cvref_t<Result> >{
      std::invoke(*std::forward<FUNCTION_IN_CONTEXT>(function),
            *std::forward<ARGUMENT_IN_CONTEXT>(argument))};
  }

  template <class FUNCTION, class... ARGUMENTS>
  static auto invoke(FUNCTION&& function, ARGUMENTS&&... arguments)
  {
    using Result = std::invoke_result_t<FUNCTION, decltype(*arguments)...>;

    if (!(arguments && ...)) {
      return std::optional<remove_cvref_t<Result> >{};
    }

    return std::optional<remove_cvref_t<Result> >{
      std::invoke(std::forward<FUNCTION>(function),
            *std::forward<ARGUMENTS>(arguments)...)};
  }
};

template <class VALUE_TYPE>
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
struct map<applicative_tag, beman::optional::optional<VALUE_TYPE> > {
  template <class VALUE>
  static auto pure(VALUE&& value)
    -> beman::optional::optional<remove_cvref_t<VALUE> >
  {
    return beman::optional::optional<remove_cvref_t<VALUE> >{
      std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  static auto apply(FUNCTION_IN_CONTEXT&& function,
            ARGUMENT_IN_CONTEXT&& argument)
  {
    using Result =
      std::invoke_result_t<decltype(*function), decltype(*argument)>;

    if (!function || !argument) {
      return beman::optional::optional<remove_cvref_t<Result> >{};
    }

    return beman::optional::optional<remove_cvref_t<Result> >{
      std::invoke(*std::forward<FUNCTION_IN_CONTEXT>(function),
            *std::forward<ARGUMENT_IN_CONTEXT>(argument))};
  }

  template <class FUNCTION, class... ARGUMENTS>
  static auto invoke(FUNCTION&& function, ARGUMENTS&&... arguments)
  {
    using Result = std::invoke_result_t<FUNCTION, decltype(*arguments)...>;

    if (!(arguments && ...)) {
      return beman::optional::optional<remove_cvref_t<Result> >{};
    }

    return beman::optional::optional<remove_cvref_t<Result> >{
      std::invoke(std::forward<FUNCTION>(function),
            *std::forward<ARGUMENTS>(arguments)...)};
  }
};

}  // close namespace smd

#endif
