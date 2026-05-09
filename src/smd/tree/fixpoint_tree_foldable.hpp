// src/smd/tree/fixpoint_tree_foldable.hpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIXPOINT_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FIXPOINT_TREE_FOLDABLE

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <variant>

namespace smd {

/** Foldable typeclass instance for Fix<ExprF> (the fixpoint expression tree).
 * fold_map applies @p f to each ExprConst leaf value and combines results
 * using the Monoid for the return type. Add and Mul nodes combine their
 * children's results left-then-right.
 */
struct FixpointTreeFoldableImpl {

    // b2c8e4f1-3a6d-4f1b-9e7c-5d2b8a4f3c91
    template <class F>
    auto fold_map(this auto &&self, F &&f,
                  const smd::fixpoint::Fix<smd::tree::ExprF> &t) {
        using smd::fixpoint::unwrap_fix;
        using smd::tree::ExprAdd;
        using smd::tree::ExprConst;
        using smd::tree::ExprMul;
        using Expr = smd::tree::Expr;

        const auto &layer = unwrap_fix(t);

        if (std::holds_alternative<ExprConst<Expr>>(layer)) {
            return std::invoke(f, std::get<ExprConst<Expr>>(layer).value);
        }

        const auto fold_children = [&](const auto &left, const auto &right) {
            auto lhs = self.fold_map(f, *left);
            auto rhs = self.fold_map(f, *right);
            using Result = std::remove_cvref_t<decltype(lhs)>;
            return smd::typeclass::monoid_v<Result>.combine(lhs, rhs);
        };

        if (std::holds_alternative<ExprAdd<Expr>>(layer)) {
            const auto &a = std::get<ExprAdd<Expr>>(layer);
            return fold_children(a.left, a.right);
        }

        const auto &m = std::get<ExprMul<Expr>>(layer);
        return fold_children(m.left, m.right);
    }
    // b2c8e4f1-3a6d-4f1b-9e7c-5d2b8a4f3c91 end
};

/** Foldable map that exposes fold_map for Fix<ExprF>. */
struct FixpointTreeFoldableMap : Foldable<FixpointTreeFoldableImpl> {
    using FixpointTreeFoldableImpl::fold_map;
};

// c4d9f2a7-6b1e-4c3f-8a5d-2e7b9c1f4a83
/** Registers FixpointTreeFoldableMap as the Foldable instance for Fix<ExprF>. */
template <>
inline constexpr auto foldable_typeclass<smd::fixpoint::Fix<smd::tree::ExprF>> =
    FixpointTreeFoldableMap{};
// c4d9f2a7-6b1e-4c3f-8a5d-2e7b9c1f4a83 end

} // namespace smd

#endif
