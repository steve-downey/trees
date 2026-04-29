#ifndef INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP

#include <smd/typeclass/typeclass_base.hpp>

#include <beman/optional/optional.hpp>

#include <concepts>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace smd {

namespace detail {

template <class FUNCTION, class... BOUND_ARGS>
struct terminating_partial {
  FUNCTION function;
  std::tuple<BOUND_ARGS...> bound_args;

  template <class NEXT_ARG>
  auto operator()(NEXT_ARG&& next_arg)
  {
    return invoke_or_extend(std::forward<NEXT_ARG>(next_arg),
                            std::index_sequence_for<BOUND_ARGS...>{});
  }

 private:
  template <class NEXT_ARG, std::size_t... IDX>
  auto invoke_or_extend(NEXT_ARG&& next_arg, std::index_sequence<IDX...>)
  {
    if constexpr (std::invocable<FUNCTION&, BOUND_ARGS&..., NEXT_ARG>) {
      return std::invoke(function,
                         std::get<IDX>(bound_args)...,
                         std::forward<NEXT_ARG>(next_arg));
    } else {
      using NEXT_PARTIAL =
        terminating_partial<FUNCTION, BOUND_ARGS..., remove_cvref_t<NEXT_ARG> >;
      return NEXT_PARTIAL{
        function,
        std::tuple_cat(std::move(bound_args),
                       std::tuple<remove_cvref_t<NEXT_ARG> >{
                         std::forward<NEXT_ARG>(next_arg)})};
    }
  }
};

template <class FUNCTION>
auto make_terminating_partial(FUNCTION&& function)
{
  using STORED_FUNCTION = remove_cvref_t<FUNCTION>;
  return terminating_partial<STORED_FUNCTION>{
    std::forward<FUNCTION>(function),
    std::tuple<>{}};
}

}  // namespace detail

// Applicative pattern invariants:
// - Minimal required operations are pure and apply.
// - invoke is derived from pure/apply via terminating partial application,
//   but an Impl may provide a custom invoke for different semantics or efficiency.
// - Derived operations (lift/ap/zip_with/discard_*) live on that object.
// - Dispatch happens through a provided object or applicative_typeclass<Concrete>.
// - Do not introduce hidden alternate semantics without a distinct map/type.

template <class Impl>
struct Applicative : protected Impl {
  using Impl::apply;
  using Impl::pure;

  // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11
  // Teaching-friendly alias for "apply pure function to effectful arguments".
  // Prefer invoke as the primary C++ spelling (std::invoke model).
  template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
  auto apply_pure(this auto&& self,
                  FUNCTION&& function,
                  FIRST_ARGUMENT&& first_argument,
                  REST_ARGUMENTS&&... rest_arguments)
  {
    return self.invoke(std::forward<FUNCTION>(function),
                       std::forward<FIRST_ARGUMENT>(first_argument),
                       std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }

  template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
  auto invoke(this auto&& self,
              FUNCTION&& function,
              FIRST_ARGUMENT&& first_argument,
              REST_ARGUMENTS&&... rest_arguments)
  {
    using SELF = remove_cvref_t<decltype(self)>;
    using IMPL_BASE =
      std::conditional_t<std::is_const_v<SELF>, const Impl, Impl>;

    if constexpr (requires(IMPL_BASE& impl) {
                    impl.invoke(std::forward<FUNCTION>(function),
                                std::forward<FIRST_ARGUMENT>(first_argument),
                                std::forward<REST_ARGUMENTS>(rest_arguments)...);
                  }) {
      return static_cast<IMPL_BASE&>(self).invoke(
        std::forward<FUNCTION>(function),
        std::forward<FIRST_ARGUMENT>(first_argument),
        std::forward<REST_ARGUMENTS>(rest_arguments)...);
    } else {
      auto lifted_function =
        self.pure(detail::make_terminating_partial(std::forward<FUNCTION>(function)));
      return self.apply_chain(
        self.ap(std::move(lifted_function), std::forward<FIRST_ARGUMENT>(first_argument)),
        std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }
  }
  // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 end

 private:
  template <class ACCUMULATED>
  auto apply_chain(this auto&&, ACCUMULATED&& accumulated)
  {
    return std::forward<ACCUMULATED>(accumulated);
  }

  template <class ACCUMULATED, class NEXT_ARGUMENT, class... REST_ARGUMENTS>
  auto apply_chain(this auto&& self,
                   ACCUMULATED&& accumulated,
                   NEXT_ARGUMENT&& next_argument,
                   REST_ARGUMENTS&&... rest_arguments)
  {
    auto next = self.ap(std::forward<ACCUMULATED>(accumulated),
                        std::forward<NEXT_ARGUMENT>(next_argument));
    if constexpr (sizeof...(REST_ARGUMENTS) == 0) {
      return next;
    } else {
      return self.apply_chain(std::move(next),
                              std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }
  }

 public:
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

  template <class APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto apply_pure_with(this auto&&,
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

  template <const auto& APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto apply_pure_with(this auto&&,
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
};

template <class VALUE_TYPE>
struct OptionalApplicativeMap : Applicative<OptionalApplicativeImpl<VALUE_TYPE> > {
  using OptionalApplicativeImpl<VALUE_TYPE>::apply;
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
};

template <class VALUE_TYPE>
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
struct BemanOptionalApplicativeMap
    : Applicative<BemanOptionalApplicativeImpl<VALUE_TYPE> > {
  using BemanOptionalApplicativeImpl<VALUE_TYPE>::apply;
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
