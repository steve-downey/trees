// src/smd/tree/fixpoint_tree_foldable.t.cpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <vector>

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;
using smd::tree::mul_expr;

namespace {

template <const auto& FOLDABLE = smd::foldable_typeclass<Expr>>
auto sum_with_nttp_lookup(const Expr& tree)
{
    return FOLDABLE.fold_left(tree, 0.0, [](double acc, double x) {
        return acc + x;
    });
}

}  // namespace

TEST_CASE("FixpointTreeFoldableTest - Length")
{
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    CHECK(foldable.length(tree) == 2U);
}

TEST_CASE("FixpointTreeFoldableTest - LengthComplex")
{
    // (1 + 2) * (3 + 4) has 4 constants
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    CHECK(foldable.length(tree) == 4U);
}

TEST_CASE("FixpointTreeFoldableTest - FoldMapSum")
{
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         const_expr(4.0));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    auto sum = foldable.fold_map([](double x) { return static_cast<int>(x); },
                                 tree);
    CHECK(sum == 7);
}

TEST_CASE("FixpointTreeFoldableTest - FoldLeft")
{
    // 1 + 2: fold_left accumulates left-to-right
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    auto result = foldable.fold_left(tree, 0.0,
        [](double acc, double x) { return acc * 10.0 + x; });
    CHECK(result == 12.0);
}

TEST_CASE("FixpointTreeFoldableTest - FoldRight")
{
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    auto result = foldable.fold_right(tree, 0.0,
        [](double x, double acc) { return x * 10.0 + acc; });
    CHECK(result == 30.0);
}

TEST_CASE("FixpointTreeFoldableTest - FoldLeftComplex")
{
    // (1 + 2) * 3: constants are 1, 2, 3
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         const_expr(3.0));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    auto sum = foldable.fold_left(tree, 0.0,
        [](double acc, double x) { return acc + x; });
    CHECK(sum == 6.0);
    CHECK(eval(tree) == 9.0);
}

TEST_CASE("FixpointTreeFoldableTest - ToVector")
{
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         const_expr(3.0));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    auto values = foldable.to_vector(tree);
    CHECK(values == std::vector<double>{1.0, 2.0, 3.0});
}

TEST_CASE("FixpointTreeFoldableTest - PredicatesAndFind")
{
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(5.0)),
                         const_expr(3.0));
    const auto& foldable = smd::foldable_typeclass<Expr>;

    CHECK(foldable.any_of(tree, [](double x) { return x == 5.0; }));
    CHECK_FALSE(foldable.any_of(tree, [](double x) { return x == 99.0; }));
    CHECK(foldable.all_of(tree, [](double x) { return x > 0.0; }));
    CHECK_FALSE(foldable.all_of(tree, [](double x) { return x > 2.0; }));
    CHECK_FALSE(foldable.empty(tree));

    auto found = foldable.find_first(tree, [](double x) { return x > 2.0; });
    REQUIRE(found.has_value());
    CHECK(*found == 5.0);
}

TEST_CASE("FixpointTreeFoldableTest - NttpLookup")
{
    auto tree = add_expr(const_expr(10.0), const_expr(20.0));
    CHECK(sum_with_nttp_lookup(tree) == 30.0);
}

TEST_CASE("FixpointTreeFoldableTest - SingleConst")
{
    auto tree = const_expr(42.0);
    const auto& foldable = smd::foldable_typeclass<Expr>;
    CHECK(foldable.length(tree) == 1U);

    auto values = foldable.to_vector(tree);
    CHECK(values == std::vector<double>{42.0});
}

TEST_CASE("FixpointTreeFoldableTest - FoldSumMatchesEval")
{
    // For a pure-addition tree, fold_left with + equals eval
    auto tree = add_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));
    const auto& foldable = smd::foldable_typeclass<Expr>;
    auto sum = foldable.fold_left(tree, 0.0,
        [](double acc, double x) { return acc + x; });
    CHECK(sum == eval(tree));
}
