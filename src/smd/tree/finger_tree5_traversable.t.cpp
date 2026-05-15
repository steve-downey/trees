// src/smd/tree/finger_tree5_traversable.t.cpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree5_traversable.hpp>
#include <smd/tree/finger_tree5_traversable.hpp> // Re-inclusion check

#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("FingerTree5TraversableTest - TraverseOptionalSucceeds")
{
    using Tree = smd::tree::FingerTree5<int>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto result =
        smd::traverse([](int x) { return std::optional{x * 10}; }, t);
    REQUIRE(result.has_value());
    CHECK(result->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FingerTree5TraversableTest - TraverseOptionalFailsOnNullopt")
{
    using Tree = smd::tree::FingerTree5<int>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto result = smd::traverse(
        [](int x) -> std::optional<int> {
            return x == 2 ? std::nullopt : std::optional{x};
        },
        t);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("FingerTree5TraversableTest - TraversePreservesShape")
{
    using Tree = smd::tree::FingerTree5<int>;
    auto t = Tree::from_sequence({10, 20, 30, 40});
    auto result = smd::traverse([](int x) { return std::optional{x + 1}; }, t);
    REQUIRE(result.has_value());
    CHECK(result->flatten() == (std::vector<int>{11, 21, 31, 41}));
}

TEST_CASE("FingerTree5TraversableTest - TraverseOverLargeTree")
{
    // Cover the spine-Node3 path: tree large enough that the spine has
    // structure when traverse materializes it.
    using Tree = smd::tree::FingerTree5<int>;
    auto t = Tree::empty();
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    auto result = smd::traverse([](int x) { return std::optional{x}; }, t);
    REQUIRE(result.has_value());
    auto v = result->flatten();
    REQUIRE(v.size() == 100U);
    for (int i = 0; i < 100; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}
