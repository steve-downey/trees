// src/smd/typeclass/test/test_support.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_TEST_TEST_SUPPORT
#define INCLUDED_SMD_TYPECLASS_TEST_TEST_SUPPORT

#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace smd::typeclass::test {

/** Return true if @p left == @p right; thin wrapper for use in law checks. */
template <class LEFT, class RIGHT>
auto are_equal(LEFT &&left, RIGHT &&right) -> bool {
    return std::forward<LEFT>(left) == std::forward<RIGHT>(right);
}

/** Verify the Applicative identity law: `pure(id) <*> v == v`.
 * @tparam CONTEXT an applicative context type with a registered typeclass
 */
template <class CONTEXT>
auto check_applicative_identity_law(const CONTEXT &value) -> bool {
    const auto &applicative =
        smd::applicative_typeclass<smd::remove_cvref_t<CONTEXT>>;
    auto result = applicative.ap(
        applicative.pure([](const auto &x) { return x; }), value);
    return result == value;
}

/** Verify the Applicative homomorphism law: `pure(f) <*> pure(x) == pure(f x)`.
 * @tparam CONTEXT the applicative context whose typeclass is under test
 */
template <class CONTEXT, class FUNCTION, class VALUE>
auto check_applicative_homomorphism_law(const FUNCTION &function,
                                        const VALUE &value) -> bool {
    const auto &applicative =
        smd::applicative_typeclass<smd::remove_cvref_t<CONTEXT>>;
    auto left =
        applicative.ap(applicative.pure(function), applicative.pure(value));
    auto right = applicative.pure(std::invoke(function, value));
    return left == right;
}

/** Verify that `invoke(f, u, v)` agrees with the desugared `ap`/`pure` form
 * for a binary @p function applied to effectful arguments @p first and @p
 * second.
 * @tparam CONTEXT the applicative context whose typeclass is under test
 */
template <class CONTEXT, class FUNCTION>
auto check_applicative_invoke_binary_law(const FUNCTION &function,
                                         const CONTEXT &first,
                                         const CONTEXT &second) -> bool {
    const auto &applicative =
        smd::applicative_typeclass<smd::remove_cvref_t<CONTEXT>>;
    auto invoke_result = applicative.invoke(function, first, second);

    auto ap_result = applicative.ap(
        applicative.ap(applicative.pure([function](const auto &lhs) {
            return [function, lhs](const auto &rhs) {
                return std::invoke(function, lhs, rhs);
            };
        }),
                       first),
        second);

    return invoke_result == ap_result;
}

/** Minimal single-element applicative context used in law tests.
 * `pure(x)` wraps @p x; `apply` unwraps and invokes the stored function.
 */
template <class VALUE_TYPE>
struct Identity {
    using value_type = VALUE_TYPE;

    VALUE_TYPE value;

    friend auto operator==(const Identity &, const Identity &)
        -> bool = default;
};

/** Like Identity but uses forwarding (rvalue-ref) apply semantics.
 * Distinguishes dispatch paths in tests that require move-only contexts.
 */
template <class VALUE_TYPE>
struct BareIdentity {
    using value_type = VALUE_TYPE;

    VALUE_TYPE value;

    friend auto operator==(const BareIdentity &, const BareIdentity &)
        -> bool = default;
};

/** Ordered multi-element foldable context backed by `std::vector`.
 * fold_map accumulates left-to-right in `values` order.
 */
template <class VALUE_TYPE>
struct Sequence {
    using value_type = VALUE_TYPE;

    std::vector<VALUE_TYPE> values;

    friend auto operator==(const Sequence &, const Sequence &)
        -> bool = default;
};

using smd::typeclass::Count;

/** Convenience alias so tests can write `Vector<int>` instead of
 * `std::vector<int>`. */
template <class VALUE_TYPE>
using Vector = std::vector<VALUE_TYPE>;

} // namespace smd::typeclass::test

namespace smd {

/** Applicative implementation for Identity<V>: pure wraps, apply unwraps and
 * invokes. */
template <class VALUE_TYPE>
struct TestIdentityApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        return smd::typeclass::test::Identity<remove_cvref_t<VALUE>>{
            std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, const FUNCTION_IN_CONTEXT &function,
               const ARGUMENT_IN_CONTEXT &argument) {
        using Result = std::invoke_result_t<
            const typename remove_cvref_t<FUNCTION_IN_CONTEXT>::value_type &,
            const typename remove_cvref_t<ARGUMENT_IN_CONTEXT>::value_type &>;

        return smd::typeclass::test::Identity<remove_cvref_t<Result>>{
            std::invoke(function.value, argument.value)};
    }
};

/** Applicative typeclass record for Identity<V>; exposes pure and apply. */
template <class VALUE_TYPE>
struct TestIdentityApplicativeMap
    : Applicative<TestIdentityApplicativeImpl<VALUE_TYPE>> {
    using TestIdentityApplicativeImpl<VALUE_TYPE>::apply;
    using TestIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
inline constexpr auto
    applicative_typeclass<smd::typeclass::test::Identity<VALUE_TYPE>> =
        TestIdentityApplicativeMap<VALUE_TYPE>{};

/** Applicative implementation for BareIdentity<V>: apply uses forwarding
 * references. */
template <class VALUE_TYPE>
struct BareIdentityApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        return smd::typeclass::test::BareIdentity<remove_cvref_t<VALUE>>{
            std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, FUNCTION_IN_CONTEXT &&function,
               ARGUMENT_IN_CONTEXT &&argument) {
        using Result = std::invoke_result_t<
            decltype(std::forward<FUNCTION_IN_CONTEXT>(function).value),
            decltype(std::forward<ARGUMENT_IN_CONTEXT>(argument).value)>;

        return smd::typeclass::test::BareIdentity<remove_cvref_t<Result>>{
            std::invoke(std::forward<FUNCTION_IN_CONTEXT>(function).value,
                        std::forward<ARGUMENT_IN_CONTEXT>(argument).value)};
    }
};

/** Applicative typeclass record for BareIdentity<V>; exposes pure and apply. */
template <class VALUE_TYPE>
struct BareIdentityApplicativeMap
    : Applicative<BareIdentityApplicativeImpl<VALUE_TYPE>> {
    using BareIdentityApplicativeImpl<VALUE_TYPE>::apply;
    using BareIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
inline constexpr auto
    applicative_typeclass<smd::typeclass::test::BareIdentity<VALUE_TYPE>> =
        BareIdentityApplicativeMap<VALUE_TYPE>{};

/** Foldable implementation for Sequence<V>: fold_map walks values
 * left-to-right. */
template <class VALUE_TYPE>
struct TestSequenceFoldableImpl {
    template <class FUNCTION>
    auto fold_map(this auto &&, FUNCTION &&function,
                  const smd::typeclass::test::Sequence<VALUE_TYPE> &sequence) {
        using Result =
            remove_cvref_t<std::invoke_result_t<FUNCTION, const VALUE_TYPE &>>;
        return std::ranges::fold_left(
            sequence.values, smd::monoid_identity<Result>(),
            [&](Result acc, const VALUE_TYPE &value) {
                return smd::monoid_combine(std::move(acc),
                                           std::invoke(function, value));
            });
    }
};

/** Foldable typeclass record for Sequence<V>; exposes fold_map. */
template <class VALUE_TYPE>
struct TestSequenceFoldableMap
    : Foldable<TestSequenceFoldableImpl<VALUE_TYPE>> {
    using TestSequenceFoldableImpl<VALUE_TYPE>::fold_map;
};

template <class VALUE_TYPE>
inline constexpr auto
    foldable_typeclass<smd::typeclass::test::Sequence<VALUE_TYPE>> =
        TestSequenceFoldableMap<VALUE_TYPE>{};

/** Traversable implementation for Identity<V>: traverse applies f to the single
 * value and wraps the result back in Identity inside the applicative effect.
 */
template <class VALUE_TYPE>
struct TestIdentityTraversableImpl {
    using element_type = VALUE_TYPE;

    template <class APPLICATIVE, class FUNCTION>
    auto traverse(this auto &&, const APPLICATIVE &applicative,
                  FUNCTION &&function,
                  const smd::typeclass::test::Identity<VALUE_TYPE> &identity) {
        return applicative.invoke(
            [](auto &&value) {
                using U = remove_cvref_t<decltype(value)>;
                return smd::typeclass::test::Identity<U>{
                    std::forward<decltype(value)>(value)};
            },
            std::invoke(std::forward<FUNCTION>(function), identity.value));
    }
};

/** Traversable typeclass record for Identity<V>; exposes traverse. */
template <class VALUE_TYPE>
struct TestIdentityTraversableMap
    : Traversable<TestIdentityTraversableImpl<VALUE_TYPE>> {
    using TestIdentityTraversableImpl<VALUE_TYPE>::traverse;
};

template <class VALUE_TYPE>
inline constexpr auto
    traversable_typeclass<smd::typeclass::test::Identity<VALUE_TYPE>> =
        TestIdentityTraversableMap<VALUE_TYPE>{};

} // namespace smd

#endif // INCLUDED_SMD_TYPECLASS_TEST_TEST_SUPPORT
