// src/smd/tree/fix_tree.t.cpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("FixTreeTest - LeafConstruction")
{
    auto t = smd::tree::FixTree<int>::leaf(7);
    CHECK(t.is_leaf());
    CHECK(t.value() == 7);
}

TEST_CASE("FixTreeTest - NodeConstruction")
{
    using Tree = smd::tree::FixTree<int>;
    auto t = Tree::node(Tree::leaf(1), Tree::leaf(2));
    CHECK_FALSE(t.is_leaf());
    CHECK(t.left().is_leaf());
    CHECK(t.left().value() == 1);
    CHECK(t.right().is_leaf());
    CHECK(t.right().value() == 2);
}

TEST_CASE("FixTreeTest - NestedNodes")
{
    using Tree = smd::tree::FixTree<int>;
    auto t = Tree::node(Tree::node(Tree::leaf(1), Tree::leaf(2)),
                        Tree::leaf(3));
    CHECK_FALSE(t.is_leaf());
    CHECK_FALSE(t.left().is_leaf());
    CHECK(t.left().left().value() == 1);
    CHECK(t.right().value() == 3);
}
