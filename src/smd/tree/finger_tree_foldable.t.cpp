// src/smd/tree/finger_tree_foldable.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_foldable.hpp>
#include <smd/tree/finger_tree_foldable.hpp> // Re-inclusion check

#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("FingerTreeFoldableTest - LengthAndToVector") {
    using Tree = smd::tree::FingerTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::from_sequence({1, 2, 3, 4, 5});
    CHECK(foldable.length(t) == 5);
    CHECK(foldable.to_vector(t) == (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("FingerTreeFoldableTest - FoldMapAccumulates") {
    using Tree = smd::tree::FingerTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto sum = foldable.fold_map([](int x) { return x; }, t);
    CHECK(sum == 6);
}

TEST_CASE("FingerTreeFoldableTest - EmptyTreeHasLengthZero") {
    using Tree = smd::tree::FingerTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::empty();
    CHECK(foldable.length(t) == 0);
    CHECK(foldable.empty(t));
    CHECK(foldable.to_vector(t) == (std::vector<int>{}));
}
