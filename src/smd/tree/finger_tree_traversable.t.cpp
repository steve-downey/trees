// src/smd/tree/finger_tree_traversable.t.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_traversable.hpp>
#include <smd/tree/finger_tree_traversable.hpp> // Re-inclusion check

#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalSucceeds") {
    using Tree = smd::tree::FingerTree<int>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto result = smd::traverse([](int x) { return std::optional{x * 10}; }, t);
    REQUIRE(result.has_value());
    CHECK(result->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalFailsOnNullopt") {
    using Tree = smd::tree::FingerTree<int>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto result = smd::traverse(
        [](int x) -> std::optional<int> {
            return x == 2 ? std::nullopt : std::optional{x};
        },
        t);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("FingerTreeTraversableTest - TraversePreservesShape") {
    using Tree = smd::tree::FingerTree<int>;
    auto t = Tree::from_sequence({10, 20, 30, 40});
    auto result = smd::traverse([](int x) { return std::optional{x + 1}; }, t);
    REQUIRE(result.has_value());
    CHECK(result->flatten() == (std::vector<int>{11, 21, 31, 41}));
}
