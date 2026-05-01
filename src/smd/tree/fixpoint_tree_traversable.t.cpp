// src/smd/tree/fixpoint_tree_traversable.t.cpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fixpoint_tree_traversable.hpp>
#include <smd/tree/fixpoint_tree_traversable.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;
using smd::tree::mul_expr;

namespace {

struct ValidatePositive {
    auto operator()(double x) const -> std::optional<double>
    {
        return x > 0.0 ? std::optional<double>{x} : std::nullopt;
    }
};

struct DoubleValue {
    auto operator()(double x) const -> std::optional<double>
    {
        return std::optional<double>{x * 2.0};
    }
};

struct IncrementValue {
    auto operator()(double x) const -> std::optional<double>
    {
        return std::optional<double>{x + 1.0};
    }
};

}  // namespace

TEST_CASE("FixpointTreeTraversableTest - TraverseOptionalSuccess")
{
    // (1 + 2) * 3 = 9, all positive
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         const_expr(3.0));

    auto traversed = smd::traverse(ValidatePositive{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 9.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraverseOptionalFailure")
{
    // (-1 + 2) * 3, -1 is not positive
    auto tree = mul_expr(add_expr(const_expr(-1.0), const_expr(2.0)),
                         const_expr(3.0));

    auto traversed = smd::traverse(ValidatePositive{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FixpointTreeTraversableTest - TraverseDoublesConstants")
{
    // (1 + 2) * 3 = 9. After doubling: (2 + 4) * 6 = 36
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         const_expr(3.0));

    auto traversed = smd::traverse(DoubleValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 36.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraverseIncrementsConstants")
{
    // (1 + 2) * 3 = 9. After +1: (2 + 3) * 4 = 20
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         const_expr(3.0));

    auto traversed = smd::traverse(IncrementValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 20.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraverseLeaf")
{
    auto tree = const_expr(7.0);

    auto traversed = smd::traverse(DoubleValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 14.0);
}

TEST_CASE("FixpointTreeTraversableTest - ForEachOptionalSuccess")
{
    auto tree = add_expr(const_expr(10.0), const_expr(20.0));
    const auto& traversable = smd::traversable_typeclass<Expr>;

    auto traversed = traversable.for_each(tree, DoubleValue{});

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 60.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraversePreservesStructure")
{
    using smd::fixpoint::unwrap;
    using smd::tree::ExprAdd;

    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    auto traversed = smd::traverse(IncrementValue{}, tree);

    REQUIRE(traversed.has_value());
    const auto& layer = unwrap(*traversed);
    CHECK(std::holds_alternative<ExprAdd<Expr>>(layer));
}

TEST_CASE("FixpointTreeTraversableTest - ExplicitObjectLookup")
{
    auto tree = mul_expr(const_expr(5.0), const_expr(6.0));
    const auto& traversable = smd::traversable_typeclass<Expr>;

    const auto& applicative =
        smd::applicative_typeclass<std::optional<double>>;
    auto traversed =
        traversable.traverse(applicative, DoubleValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 120.0);
}
