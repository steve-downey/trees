#ifndef INCLUDE_SMD_TYPECLASS_TEST_TEST_SUPPORT_HPP
#define INCLUDE_SMD_TYPECLASS_TEST_TEST_SUPPORT_HPP

#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <utility>
#include <vector>

namespace smd::typeclass::test {

template <class LEFT, class RIGHT>
auto are_equal(LEFT&& left, RIGHT&& right) -> bool
{
    return std::forward<LEFT>(left) == std::forward<RIGHT>(right);
}

template <class VALUE_TYPE>
struct Identity {
    using value_type = VALUE_TYPE;

    VALUE_TYPE value;

    friend auto operator==(const Identity&, const Identity&) -> bool = default;
};

  template <class VALUE_TYPE>
  struct BareIdentity {
    using value_type = VALUE_TYPE;

    VALUE_TYPE value;

    friend auto operator==(const BareIdentity&, const BareIdentity&) -> bool = default;
  };

template <class VALUE_TYPE>
struct Sequence {
    using value_type = VALUE_TYPE;

    std::vector<VALUE_TYPE> values;

    friend auto operator==(const Sequence&, const Sequence&) -> bool = default;
};

using smd::typeclass::Count;

template <class VALUE_TYPE>
using Vector = std::vector<VALUE_TYPE>;

}  // close namespace smd::typeclass::test

namespace smd {

template <class VALUE_TYPE>
struct TestIdentityApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    return smd::typeclass::test::Identity<remove_cvref_t<VALUE> >{
      std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(this auto&&,
             const FUNCTION_IN_CONTEXT& function,
             const ARGUMENT_IN_CONTEXT& argument)
  {
    using Result = std::invoke_result_t<
      const typename remove_cvref_t<FUNCTION_IN_CONTEXT>::value_type&,
      const typename remove_cvref_t<ARGUMENT_IN_CONTEXT>::value_type&>;

    return smd::typeclass::test::Identity<remove_cvref_t<Result> >{
      std::invoke(function.value, argument.value)};
  }
};

template <class VALUE_TYPE>
struct TestIdentityApplicativeMap
    : Applicative<TestIdentityApplicativeImpl<VALUE_TYPE> > {
  using TestIdentityApplicativeImpl<VALUE_TYPE>::apply;
  using TestIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
inline constexpr auto applicative_typeclass<smd::typeclass::test::Identity<VALUE_TYPE> > =
    TestIdentityApplicativeMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
struct BareIdentityApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    return smd::typeclass::test::BareIdentity<remove_cvref_t<VALUE> >{
      std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(this auto&&,
             FUNCTION_IN_CONTEXT&& function,
             ARGUMENT_IN_CONTEXT&& argument)
  {
    using Result = std::invoke_result_t<
      decltype(std::forward<FUNCTION_IN_CONTEXT>(function).value),
      decltype(std::forward<ARGUMENT_IN_CONTEXT>(argument).value)>;

    return smd::typeclass::test::BareIdentity<remove_cvref_t<Result> >{
      std::invoke(std::forward<FUNCTION_IN_CONTEXT>(function).value,
                  std::forward<ARGUMENT_IN_CONTEXT>(argument).value)};
  }
};

template <class VALUE_TYPE>
struct BareIdentityApplicativeMap
    : Applicative<BareIdentityApplicativeImpl<VALUE_TYPE> > {
  using BareIdentityApplicativeImpl<VALUE_TYPE>::apply;
  using BareIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
inline constexpr auto applicative_typeclass<smd::typeclass::test::BareIdentity<VALUE_TYPE> > =
    BareIdentityApplicativeMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
struct TestSequenceFoldableImpl {
  template <class FUNCTION>
  auto fold_map(this auto&&,
                FUNCTION&& function,
                const smd::typeclass::test::Sequence<VALUE_TYPE>& sequence)
  {
    using Result = remove_cvref_t<std::invoke_result_t<FUNCTION, const VALUE_TYPE&> >;
    auto result = smd::monoid_identity<Result>();

    for (const auto& value : sequence.values) {
      result = smd::monoid_combine(
        std::move(result),
        std::invoke(function, value));
    }

    return result;
  }
};

template <class VALUE_TYPE>
struct TestSequenceFoldableMap : Foldable<TestSequenceFoldableImpl<VALUE_TYPE> > {
  using TestSequenceFoldableImpl<VALUE_TYPE>::fold_map;
};

template <class VALUE_TYPE>
inline constexpr auto foldable_typeclass<smd::typeclass::test::Sequence<VALUE_TYPE> > =
    TestSequenceFoldableMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
struct TestIdentityTraversableImpl {
  template <class FUNCTION>
  auto traverse(this auto&&,
                FUNCTION&& function,
                const smd::typeclass::test::Identity<VALUE_TYPE>& identity)
    -> decltype(smd::applicative_typeclass<
          remove_cvref_t<std::invoke_result_t<FUNCTION, const VALUE_TYPE&> >>.invoke(
      [](auto&& value) {
        using U = remove_cvref_t<decltype(value)>;
        return smd::typeclass::test::Identity<U>{
          std::forward<decltype(value)>(value)};
      },
      std::invoke(std::forward<FUNCTION>(function), identity.value)))
  {
    auto lifted = std::invoke(std::forward<FUNCTION>(function), identity.value);
    using Context = remove_cvref_t<decltype(lifted)>;
    const auto& applicative = smd::applicative_typeclass<Context>;

    return applicative.invoke(
      [](auto&& value) {
        using U = remove_cvref_t<decltype(value)>;
        return smd::typeclass::test::Identity<U>{
          std::forward<decltype(value)>(value)};
      },
      lifted);
  }
};

template <class VALUE_TYPE>
struct TestIdentityTraversableMap
    : Traversable<TestIdentityTraversableImpl<VALUE_TYPE> > {
  using TestIdentityTraversableImpl<VALUE_TYPE>::traverse;
};

template <class VALUE_TYPE>
inline constexpr auto traversable_typeclass<smd::typeclass::test::Identity<VALUE_TYPE> > =
    TestIdentityTraversableMap<VALUE_TYPE>{};

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_TEST_TEST_SUPPORT_HPP
