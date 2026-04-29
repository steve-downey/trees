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
// - Instances are single lookup objects that provide pure/apply/invoke.
// - Optional derived operations (lift/ap/zip_with/discard_*) live on that object.
// - Dispatch happens through a provided object or applicative_typeclass<Concrete>.
// - Do not introduce hidden alternate semantics without a distinct map/type.

template <class Impl>
struct Applicative : protected Impl {
  using Impl::apply;
  using Impl::invoke;
  using Impl::pure;

  template <class FUNCTION, class ARGUMENT>
  auto map(this auto&& self, FUNCTION&& function, ARGUMENT&& argument)
  {
    return self.invoke(std::forward<FUNCTION>(function),
                       std::forward<ARGUMENT>(argument));
  }

  template <class VALUE>
  auto lift(this auto&& self, VALUE&& value)
  {
    return self.pure(std::forward<VALUE>(value));
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto ap(this auto&& self,
          FUNCTION_IN_CONTEXT&& function,
          ARGUMENT_IN_CONTEXT&& argument)
  {
    return self.apply(std::forward<FUNCTION_IN_CONTEXT>(function),
                      std::forward<ARGUMENT_IN_CONTEXT>(argument));
  }

  template <class FUNCTION, class FIRST_ARGUMENT, class SECOND_ARGUMENT>
  auto zip_with(this auto&& self,
                FUNCTION&& function,
                FIRST_ARGUMENT&& first_argument,
                SECOND_ARGUMENT&& second_argument)
  {
    return self.invoke(std::forward<FUNCTION>(function),
                       std::forward<FIRST_ARGUMENT>(first_argument),
                       std::forward<SECOND_ARGUMENT>(second_argument));
  }

  template <class FIRST_ARGUMENT, class SECOND_ARGUMENT>
  auto discard_first(this auto&& self,
                     FIRST_ARGUMENT&& first_argument,
                     SECOND_ARGUMENT&& second_argument)
  {
    return self.invoke(
      [](const auto&, auto&& rhs) {
        return std::forward<decltype(rhs)>(rhs);
      },
      std::forward<FIRST_ARGUMENT>(first_argument),
      std::forward<SECOND_ARGUMENT>(second_argument));
  }

  template <class FIRST_ARGUMENT, class SECOND_ARGUMENT>
  auto discard_second(this auto&& self,
                      FIRST_ARGUMENT&& first_argument,
                      SECOND_ARGUMENT&& second_argument)
  {
    return self.invoke(
      [](auto&& lhs, const auto&) {
        return std::forward<decltype(lhs)>(lhs);
      },
      std::forward<FIRST_ARGUMENT>(first_argument),
      std::forward<SECOND_ARGUMENT>(second_argument));
  }

  template <class APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto invoke_with(this auto&&,
                   const APPLICATIVE_MAP& applicative_map,
                   FUNCTION&& function,
                   FIRST_ARGUMENT&& first_argument,
                   REST_ARGUMENTS&&... rest_arguments)
  {
    return applicative_map.invoke(std::forward<FUNCTION>(function),
                                  std::forward<FIRST_ARGUMENT>(first_argument),
                                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }

  template <const auto& APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto invoke_with(this auto&&,
                   FUNCTION&& function,
                   FIRST_ARGUMENT&& first_argument,
                   REST_ARGUMENTS&&... rest_arguments)
  {
    return APPLICATIVE_MAP.invoke(std::forward<FUNCTION>(function),
                                  std::forward<FIRST_ARGUMENT>(first_argument),
                                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }
};

template <class T>
inline constexpr auto applicative_typeclass = std::false_type{};

template <class VALUE_TYPE>
struct OptionalApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value) -> std::optional<remove_cvref_t<VALUE> >
  {
    return std::optional<remove_cvref_t<VALUE> >{std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(this auto&&,
             FUNCTION_IN_CONTEXT&& function,
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
  auto invoke(this auto&&, FUNCTION&& function, ARGUMENTS&&... arguments)
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
struct OptionalApplicativeMap : Applicative<OptionalApplicativeImpl<VALUE_TYPE> > {
  using OptionalApplicativeImpl<VALUE_TYPE>::apply;
  using OptionalApplicativeImpl<VALUE_TYPE>::invoke;
  using OptionalApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
struct BemanOptionalApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
    -> beman::optional::optional<remove_cvref_t<VALUE> >
  {
    return beman::optional::optional<remove_cvref_t<VALUE> >{
      std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(this auto&&,
             FUNCTION_IN_CONTEXT&& function,
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
  auto invoke(this auto&&, FUNCTION&& function, ARGUMENTS&&... arguments)
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
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
struct BemanOptionalApplicativeMap
    : Applicative<BemanOptionalApplicativeImpl<VALUE_TYPE> > {
  using BemanOptionalApplicativeImpl<VALUE_TYPE>::apply;
  using BemanOptionalApplicativeImpl<VALUE_TYPE>::invoke;
  using BemanOptionalApplicativeImpl<VALUE_TYPE>::pure;
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
