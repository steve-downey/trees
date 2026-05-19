// src/smd/typeclass/dual_monoid.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_DUAL_MONOID
#define INCLUDED_SMD_TYPECLASS_DUAL_MONOID

// DualMonoid<M> wraps a monoidal value M and flips the argument order of
// combine.  The identity element is the same as M's identity.
//
// This is the standard algebraic dual: if (M, ·, e) is a monoid then
// (M, ·ᵒᵖ, e) where a ·ᵒᵖ b = b · a is also a monoid.
//
// Primary use: FingerTree5::reversed() returns a tree whose measure tag is
// DualMonoid<Tag>, ensuring that prefix-accumulated measures grow from the
// new left edge (the old right edge) under the correct combine order.
//
// split_at_measure requires >= on the Tag type; DualMonoid<M> forwards >=
// from M so that split operations work on reversed trees.

#include <smd/typeclass/monoid.hpp>

#include <concepts>

namespace smd::typeclass {

template <typename M>
struct DualMonoid {
    M value;
    friend auto operator==(const DualMonoid &, const DualMonoid &)
        -> bool = default;

    // Forward >= from M so split_at_measure compiles on reversed trees.
    friend auto operator>=(const DualMonoid &lhs, const DualMonoid &rhs) -> bool
        requires requires(const M &a, const M &b) {
            { a >= b } -> std::convertible_to<bool>;
        }
    {
        return lhs.value >= rhs.value;
    }
};

template <typename M>
struct Monoid<DualMonoid<M>> {
    constexpr auto identity() const -> DualMonoid<M> {
        return DualMonoid<M>{monoid_v<M>.identity()};
    }
    constexpr auto combine(const DualMonoid<M> &lhs,
                           const DualMonoid<M> &rhs) const -> DualMonoid<M> {
        // Flip the argument order — that's the entire mechanism.
        return DualMonoid<M>{monoid_v<M>.combine(rhs.value, lhs.value)};
    }
};

} // namespace smd::typeclass

#endif
