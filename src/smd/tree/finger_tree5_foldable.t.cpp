// src/smd/tree/finger_tree5_foldable.t.cpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree5_foldable.hpp>
#include <smd/tree/finger_tree5_foldable.hpp> // Re-inclusion check

#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("FingerTree5FoldableTest - LengthAndToVector")
{
    using Tree = smd::tree::FingerTree5<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::from_sequence({1, 2, 3, 4, 5});
    CHECK(foldable.length(t) == 5);
    CHECK(foldable.to_vector(t) == (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("FingerTree5FoldableTest - FoldMapAccumulates")
{
    using Tree = smd::tree::FingerTree5<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto sum = foldable.fold_map([](int x) { return x; }, t);
    CHECK(sum == 6);
}

TEST_CASE("FingerTree5FoldableTest - EmptyTreeHasLengthZero")
{
    using Tree = smd::tree::FingerTree5<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::empty();
    CHECK(foldable.length(t) == 0);
    CHECK(foldable.empty(t));
    CHECK(foldable.to_vector(t) == (std::vector<int>{}));
}

TEST_CASE("FingerTree5FoldableTest - LargeTreeWalksAllLeaves")
{
    // Ensures the recursive Elem unpack inside for_each reaches every leaf
    // even after the spine takes on Node3 structure.
    using Tree = smd::tree::FingerTree5<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;

    auto t = Tree::empty();
    for (int i = 1; i <= 100; ++i)
        t = t.snoc(i);

    auto sum = foldable.fold_map([](int x) { return x; }, t);
    CHECK(sum == 5050);
    CHECK(foldable.length(t) == 100);
}

TEST_CASE("FingerTree5FoldableTest - StringFoldMap")
{
    using Tree = smd::tree::FingerTree5<std::string>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::from_sequence({"a", "b", "c", "d"});
    auto joined = foldable.fold_map([](const std::string &s) { return s; }, t);
    CHECK(joined == "abcd");
}
