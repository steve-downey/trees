// src/smd/typeclass/examples/blog_fixpoint_examples.cpp              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree_traversable.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <cstddef>
#include <optional>
#include <vector>

namespace smd::typeclass::examples {

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;
using smd::tree::mul_expr;

// blog-foldable-showcase-begin
auto blog_foldable_showcase() -> bool {
    const auto &foldable = smd::foldable_typeclass<Expr>;

    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));

    if (foldable.length(tree) != 4U) {
        return false;
    }

    auto values = foldable.to_vector(tree);
    if (values != std::vector<double>{1.0, 2.0, 3.0, 4.0}) {
        return false;
    }

    auto sum = foldable.fold_left(tree, 0.0,
                                  [](double acc, double x) { return acc + x; });
    if (sum != 10.0) {
        return false;
    }

    if (!foldable.any_of(tree, [](double x) { return x == 3.0; })) {
        return false;
    }

    if (!foldable.all_of(tree, [](double x) { return x > 0.0; })) {
        return false;
    }

    auto found = foldable.find_first(tree, [](double x) { return x > 2.0; });
    if (!found.has_value() || *found != 3.0) {
        return false;
    }

    return true;
}
// blog-foldable-showcase-end

// blog-traverse-validate-begin
auto blog_traverse_validate() -> bool {
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    auto validated = smd::traverse(
        [](double x) -> std::optional<double> {
            return x > 0.0 ? std::optional{x} : std::nullopt;
        },
        tree);

    if (!validated.has_value()) {
        return false;
    }
    if (eval(*validated) != 9.0) {
        return false;
    }

    auto bad_tree =
        mul_expr(add_expr(const_expr(-1.0), const_expr(2.0)), const_expr(3.0));

    auto bad_result = smd::traverse(
        [](double x) -> std::optional<double> {
            return x > 0.0 ? std::optional{x} : std::nullopt;
        },
        bad_tree);

    return !bad_result.has_value();
}
// blog-traverse-validate-end

// blog-traverse-relabel-begin
auto blog_traverse_relabel() -> bool {
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    auto doubled = smd::traverse(
        [](double x) -> std::optional<double> {
            return std::optional{x * 2.0};
        },
        tree);

    if (!doubled.has_value()) {
        return false;
    }

    return eval(*doubled) == 36.0;
}
// blog-traverse-relabel-end

} // namespace smd::typeclass::examples
