// src/smd/tree/fixpoint_tree_traversable.hpp                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIXPOINT_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FIXPOINT_TREE_TRAVERSABLE

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <utility>
#include <variant>

namespace smd {

/** Traversable typeclass instance for Fix<ExprF> (the fixpoint expression
 * tree). Treats ExprConst leaf doubles as the traversal element type. traverse
 * maps each constant value through the applicative and rebuilds an Expr in that
 * context; Add and Mul nodes combine their traversed children with the
 * appropriate builder (add_expr / mul_expr) inside the applicative.
 */
struct FixpointTreeTraversableImpl {
    using element_type = double;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative, F &&f,
                  const smd::fixpoint::Fix<smd::tree::ExprF> &t) {
        using smd::fixpoint::unwrap_fix;
        using smd::tree::ExprAdd;
        using smd::tree::ExprConst;
        using smd::tree::ExprMul;
        using Expr = smd::tree::Expr;

        const auto &layer = unwrap_fix(t);

        if (std::holds_alternative<ExprConst<Expr>>(layer)) {
            return applicative.invoke(
                [](double value) { return smd::tree::const_expr(value); },
                std::invoke(std::forward<F>(f),
                            std::get<ExprConst<Expr>>(layer).value));
        }

        const auto traverse_pair = [&](const auto &left, const auto &right,
                                       auto builder) {
            auto l = self.traverse(applicative, f, *left);
            auto r = self.traverse(applicative, f, *right);
            return applicative.invoke(std::move(builder), l, r);
        };

        if (std::holds_alternative<ExprAdd<Expr>>(layer)) {
            const auto &a = std::get<ExprAdd<Expr>>(layer);
            return traverse_pair(a.left, a.right, [](auto &&l, auto &&r) {
                return smd::tree::add_expr(std::forward<decltype(l)>(l),
                                           std::forward<decltype(r)>(r));
            });
        }

        const auto &m = std::get<ExprMul<Expr>>(layer);
        return traverse_pair(m.left, m.right, [](auto &&l, auto &&r) {
            return smd::tree::mul_expr(std::forward<decltype(l)>(l),
                                       std::forward<decltype(r)>(r));
        });
    }
};

/** Traversable map that exposes traverse for Fix<ExprF>. */
struct FixpointTreeTraversableMap : Traversable<FixpointTreeTraversableImpl> {
    using FixpointTreeTraversableImpl::traverse;
};

/** Registers FixpointTreeTraversableMap as the Traversable instance for
 * Fix<ExprF>. */
template <>
inline constexpr auto
    traversable_typeclass<smd::fixpoint::Fix<smd::tree::ExprF>> =
        FixpointTreeTraversableMap{};

} // namespace smd

#endif
