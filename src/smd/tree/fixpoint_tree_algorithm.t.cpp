// src/smd/tree/fixpoint_tree_algorithm.t.cpp                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fixpoint_tree_algorithm.hpp>
#include <smd/tree/fixpoint_tree_algorithm.hpp> // Re-inclusion check

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;
using smd::tree::mul_expr;

namespace algorithm = smd::tree::algorithm;

TEST_CASE("Validate - AllPass") {
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    auto result = algorithm::validate([](double x) { return x > 0.0; }, tree);

    REQUIRE(result.has_value());
    CHECK(eval(*result) == 9.0);
}

TEST_CASE("Validate - OneFails") {
    auto tree =
        mul_expr(add_expr(const_expr(-1.0), const_expr(2.0)), const_expr(3.0));

    auto result = algorithm::validate([](double x) { return x > 0.0; }, tree);

    CHECK_FALSE(result.has_value());
}

TEST_CASE("Validate - AllLeavesFail") {
    auto tree = add_expr(const_expr(-1.0), const_expr(-2.0));

    auto result = algorithm::validate([](double x) { return x > 0.0; }, tree);

    CHECK_FALSE(result.has_value());
}

TEST_CASE("Validate - SingleLeafPass") {
    auto tree = const_expr(42.0);

    auto result = algorithm::validate([](double x) { return x > 0.0; }, tree);

    REQUIRE(result.has_value());
    CHECK(eval(*result) == 42.0);
}

TEST_CASE("TransformIfLarge - LargeEnough") {
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));

    auto result = algorithm::transform_if_large(
        3, [](double x) { return x * 10.0; }, tree);

    REQUIRE(result.has_value());
    CHECK(eval(*result) == 2100.0);
}

TEST_CASE("TransformIfLarge - TooSmall") {
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));

    auto result = algorithm::transform_if_large(
        5, [](double x) { return x * 10.0; }, tree);

    CHECK_FALSE(result.has_value());
}

TEST_CASE("TransformIfLarge - ExactThreshold") {
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));

    auto result = algorithm::transform_if_large(
        2, [](double x) { return x + 10.0; }, tree);

    REQUIRE(result.has_value());
    CHECK(eval(*result) == 23.0);
}

TEST_CASE("TransformIfLarge - BelowThresholdByOne") {
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));

    auto result = algorithm::transform_if_large(
        3, [](double x) { return x + 10.0; }, tree);

    CHECK_FALSE(result.has_value());
}
