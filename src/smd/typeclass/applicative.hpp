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

// Applicative pattern invariants:
// - Generic entry points pure/apply/invoke dispatch via applicative_typeclass<T>.
// - Instances provide an object with coherent pure/apply/invoke and specialize
//   applicative_typeclass<Concrete>.
// - invoke is the user-facing n-ary operation and should remain policy-explicit.
// - Do not introduce hidden alternate semantics without a distinct map/type.

template <class T>
inline constexpr auto applicative_typeclass = std::false_type{};

template <class APPLICATIVE_MAP, class VALUE>
auto pure(const APPLICATIVE_MAP& applicative_map, VALUE&& value)
{
    return applicative_map.pure(std::forward<VALUE>(value));
}

template <class CONTEXT, class VALUE>
auto pure(VALUE&& value)
{
    using ContextType = remove_cvref_t<CONTEXT>;
    return pure(applicative_typeclass<ContextType>, std::forward<VALUE>(value));
}

template <class APPLICATIVE_MAP, class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
auto apply(const APPLICATIVE_MAP& applicative_map,
           FUNCTION_IN_CONTEXT&& function,
           ARGUMENT_IN_CONTEXT&& argument)
{
    return applicative_map.apply(std::forward<FUNCTION_IN_CONTEXT>(function),
                                 std::forward<ARGUMENT_IN_CONTEXT>(argument));
}

template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
auto apply(FUNCTION_IN_CONTEXT&& function, ARGUMENT_IN_CONTEXT&& argument)
{
    using FunctionContext = remove_cvref_t<FUNCTION_IN_CONTEXT>;
    return apply(applicative_typeclass<FunctionContext>,
                 std::forward<FUNCTION_IN_CONTEXT>(function),
                 std::forward<ARGUMENT_IN_CONTEXT>(argument));
}

template <class APPLICATIVE_MAP, class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
auto invoke(const APPLICATIVE_MAP& applicative_map,
            FUNCTION&& function,
            FIRST_ARGUMENT&& first_argument,
            REST_ARGUMENTS&&... rest_arguments)
{
    return applicative_map.invoke(std::forward<FUNCTION>(function),
                                  std::forward<FIRST_ARGUMENT>(first_argument),
                                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
}

template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
auto invoke(FUNCTION&& function,
      FIRST_ARGUMENT&& first_argument,
      REST_ARGUMENTS&&... rest_arguments)
{
    using ContextType = remove_cvref_t<FIRST_ARGUMENT>;
    return invoke(applicative_typeclass<ContextType>,
                  std::forward<FUNCTION>(function),
                  std::forward<FIRST_ARGUMENT>(first_argument),
                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
}

template <class VALUE_TYPE>
struct OptionalApplicativeMap {
  template <class VALUE>
  auto pure(VALUE&& value) const -> std::optional<remove_cvref_t<VALUE> >
  {
    return std::optional<remove_cvref_t<VALUE> >{std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(FUNCTION_IN_CONTEXT&& function,
            ARGUMENT_IN_CONTEXT&& argument)
      const
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
  auto invoke(FUNCTION&& function, ARGUMENTS&&... arguments) const
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
struct BemanOptionalApplicativeMap {
  template <class VALUE>
  auto pure(VALUE&& value) const
    -> beman::optional::optional<remove_cvref_t<VALUE> >
  {
    return beman::optional::optional<remove_cvref_t<VALUE> >{
      std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(FUNCTION_IN_CONTEXT&& function,
            ARGUMENT_IN_CONTEXT&& argument)
      const
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
  auto invoke(FUNCTION&& function, ARGUMENTS&&... arguments) const
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

template <class VALUE_TYPE>
inline constexpr auto applicative_typeclass<std::optional<VALUE_TYPE> > =
    OptionalApplicativeMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
inline constexpr auto applicative_typeclass<beman::optional::optional<VALUE_TYPE> > =
    BemanOptionalApplicativeMap<VALUE_TYPE>{};

}  // close namespace smd

#endif
