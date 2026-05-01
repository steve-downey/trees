// src/smd/tree/fringe_tree_foldable.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>  // Re-inclusion check

#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeFoldableTest - SingleLeafLength")
{
    using Tree = smd::tree::FringeTree<int>;
    const auto& foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::leaf(42);
    CHECK(foldable.length(t) == 1);
    CHECK(foldable.to_vector(t) == (std::vector<int>{42}));
}

TEST_CASE("FringeTreeFoldableTest - BranchToVector")
{
    using Tree = smd::tree::FringeTree<int>;
    const auto& foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    CHECK(foldable.length(t) == 3);
    CHECK(foldable.to_vector(t) == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FringeTreeFoldableTest - EmptyTree")
{
    using Tree = smd::tree::FringeTree<int>;
    const auto& foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::empty();
    CHECK(foldable.length(t) == 0);
    CHECK(foldable.empty(t));
}
