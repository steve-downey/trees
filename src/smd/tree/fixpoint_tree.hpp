// src/smd/tree/fixpoint_tree.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIXPOINT_TREE
#define INCLUDED_SMD_TREE_FIXPOINT_TREE

#include <smd/fixpoint/box.hpp>
#include <smd/fixpoint/cata.hpp>
#include <smd/fixpoint/fix.hpp>
#include <smd/fixpoint/overloaded.hpp>

#include <functional>
#include <utility>
#include <variant>

namespace smd::tree {

using smd::fixpoint::Box;
using smd::fixpoint::make_box;

template <typename A>
struct ExprConst {
    double value;
};

template <typename A>
struct ExprAdd {
    Box<A> left;
    Box<A> right;
};

template <typename A>
struct ExprMul {
    Box<A> left;
    Box<A> right;
};

template <typename A>
using ExprF = std::variant<ExprConst<A>, ExprAdd<A>, ExprMul<A>>;

template <typename F, typename A>
auto fmap_expr(F&& f, const ExprF<A>& expr)
{
    using B = std::invoke_result_t<F, const A&>;
    return std::visit(
        smd::fixpoint::overloaded{
            [](const ExprConst<A>& c) -> ExprF<B> {
                return ExprConst<B>{c.value};
            },
            [&f](const ExprAdd<A>& a) -> ExprF<B> {
                return ExprAdd<B>{make_box<B>(std::invoke(f, *a.left)),
                                  make_box<B>(std::invoke(f, *a.right))};
            },
            [&f](const ExprMul<A>& m) -> ExprF<B> {
                return ExprMul<B>{make_box<B>(std::invoke(f, *m.left)),
                                  make_box<B>(std::invoke(f, *m.right))};
            },
        },
        expr);
}

inline constexpr auto fmap_expr_fn = [](auto&& f, const auto& expr) {
    return fmap_expr(std::forward<decltype(f)>(f), expr);
};

using Expr = smd::fixpoint::Fix<ExprF>;

inline auto const_expr(double value) -> Expr
{
    return smd::fixpoint::wrap<ExprF>(ExprF<Expr>{ExprConst<Expr>{value}});
}

inline auto add_expr(Expr left, Expr right) -> Expr
{
    return smd::fixpoint::wrap<ExprF>(ExprF<Expr>{
        ExprAdd<Expr>{make_box<Expr>(std::move(left)),
                      make_box<Expr>(std::move(right))}});
}

inline auto mul_expr(Expr left, Expr right) -> Expr
{
    return smd::fixpoint::wrap<ExprF>(ExprF<Expr>{
        ExprMul<Expr>{make_box<Expr>(std::move(left)),
                      make_box<Expr>(std::move(right))}});
}

// e3a7f1c2-9b4d-4e2a-8f6c-1d5b3a9e7c04
inline auto eval_algebra(const ExprF<double>& expr) -> double
{
    return std::visit(
        smd::fixpoint::overloaded{
            [](const ExprConst<double>& c) { return c.value; },
            [](const ExprAdd<double>& a) { return *a.left + *a.right; },
            [](const ExprMul<double>& m) { return *m.left * *m.right; },
        },
        expr);
}

inline auto eval(const Expr& expr) -> double
{
    return smd::fixpoint::cata<double>(eval_algebra, fmap_expr_fn, expr);
}
// e3a7f1c2-9b4d-4e2a-8f6c-1d5b3a9e7c04 end

}  // close namespace smd::tree

#endif
